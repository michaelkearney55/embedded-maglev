#include "train.h"
#include "watch_dog.h"

// Testing must be undefined when Arduino is not connected over USB because TESTING enables Serial communication over USB for debugging purposes.
#define TESTING

static volatile int counter = 0;

// DO NOT call begin() on Serial4. We must configure the PMUX registers manually because the Arduino library does not support SERCOM multiplexing on non-RX/TX labelled pins.
Uart Serial4(&sercom4, PIN_RX, PIN_TX, PAD_RX, PAD_TX);

// OVERWRITE SERCOM ISR TO PROCESS DATAGRAM UART
void SERCOM4_Handler() {
  if (SERCOM4->USART.INTFLAG.bit.RXC) {
    if (SERCOM4->USART.INTFLAG.bit.ERROR) {
      // Ignore the incoming message if there is an error
      SERCOM4->USART.STATUS.reg = SERCOM_USART_STATUS_RESETVALUE;
      SERCOM4->USART.INTFLAG.bit.ERROR = 1;
    } else {
      byte b = SERCOM4->USART.DATA.bit.DATA;
      #ifdef TESTING
      Serial.print("Message: ");
      Serial.print((int) b, BIN);
      Serial.print(";\tSpeed: ");
      Serial.print(maskSpeed(b));
      Serial.print(";\tDAC output: ");
      #endif

      /* 8 bit message protocol review (see train.h for in depth explanation)
       * Speed: [0:5]
       * Direction: [6]
       * Brake: [7]
       */

      // We received a message. Handle the message
      if (maskBrake(b)) {
        // disable hbridge if message indicates brake
        analogWrite(enA, 0);
        analogWrite(enB, 0);
        writeLights(false, true, false, true); // red lights
        #ifdef TESTING
        Serial.println('0');
        #endif
      } else {
        // enable hbridge in appropriate direction
        if (maskDirection(b)) { // if forward
          digitalWrite(in1, HIGH);
          digitalWrite(in2, LOW);
          digitalWrite(in3, HIGH);
          digitalWrite(in4, LOW);
          writeLights(true, false, false, true); // forward lights
        } else { // if backward
          digitalWrite(in1, LOW);
          digitalWrite(in2, HIGH);
          digitalWrite(in3, LOW);
          digitalWrite(in4, HIGH);
          writeLights(false, true, true, false); // backward lights
        }
        // Write the speed value mapped to DAC value to the motor enabler
        byte speed = maskSpeed(b);
        if (speed) { // if not 0
          // convert 1-63 to 130-255;
          // calibrated motor min and max of 130/255 just happen to be extremely convenient for bit math, so avoid map() func call
          speed = (speed << 1) | 0x80;
        }
        analogWrite(enA, speed);
        analogWrite(enB, speed);
        #ifdef TESTING
        Serial.println(speed);
        #endif
      }

      // Pet watchdog if we received the message and there was no error flag in the INTFLAG register
      WDT->CLEAR.reg = 0xA5;
      
      // clear interrupt register
      SERCOM4->USART.INTFLAG.bit.RXC = 1;
    }
  }

  // discard error interrupts, as implemented in Arduino library, because we only error check upon message reception flag
  if (SERCOM4->USART.INTFLAG.bit.ERROR || SERCOM4->USART.INTFLAG.bit.DRE) {
      SERCOM4->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
  }
}

void setup() {

  #ifdef TESTING
  Serial.begin(9600);
  while(!Serial);
  #endif

  // Set up onboard LEDs
  pinMode(aWhiteLEDPin, OUTPUT);
  pinMode(aRedLEDPin, OUTPUT);
  pinMode(bWhiteLEDPin, OUTPUT);
  pinMode(bRedLEDPin, OUTPUT);

  digitalWrite(aWhiteLEDPin, LOW);
  digitalWrite(aRedLEDPin, LOW);
  digitalWrite(bWhiteLEDPin, LOW);
  digitalWrite(bRedLEDPin, LOW);

  // // Set up motor and H Bridge
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Turn off motors - Initial state
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  
  // Arduino library does not support SERCOM pmuxing of pins other than 13/14, and we cannot overwrite the Serial1 handler.
  // As such, we need to manually configure other pins for SERCOM so we can write a custom SERCOM Handler.
  
  // BEGIN PMUXED DUPE OF UART begin()
  // Configure PIN_TX (5, PB11, SERCOM4, PAD2) as output in PORT as SERCOM-ALT with PMUX function D
  uint8_t temp = PORT->Group[PIN_TX_PORT].PMUX[PIN_TX_PORT_PIN >> 1].reg & (PIN_TX_PORT_PIN & 0x1 ? PORT_PMUX_PMUXE_Msk : PORT_PMUX_PMUXO_Msk);
  PORT->Group[PIN_TX_PORT].PMUX[PIN_TX_PORT_PIN >> 1].reg = temp | (PIN_TX_PORT_PIN & 0x1 ? PORT_PMUX_PMUXO_D : PORT_PMUX_PMUXE_D);
  PORT->Group[PIN_TX_PORT].PINCFG[PIN_TX_PORT_PIN].reg |= PORT_PINCFG_PMUXEN;

  // Configure PIN_RX (4, PB10, SERCOM4, PAD3) as input in PORT as SERCOM-ALT with PMUX function D
  temp = PORT->Group[PIN_RX_PORT].PMUX[PIN_RX_PORT_PIN >> 1].reg & (PIN_RX_PORT_PIN & 0x1 ? PORT_PMUX_PMUXE_Msk : PORT_PMUX_PMUXO_Msk);
  PORT->Group[PIN_RX_PORT].PMUX[PIN_RX_PORT_PIN >> 1].reg = temp | (PIN_RX_PORT_PIN & 0x1 ? PORT_PMUX_PMUXO_D : PORT_PMUX_PMUXE_D);
  PORT->Group[PIN_RX_PORT].PINCFG[PIN_RX_PORT_PIN].reg |= PORT_PINCFG_PMUXEN;

  // configure clock
  sercom4.initUART(UART_INT_CLOCK, SAMPLE_RATE_x16, baudrate);
  // HC-05 configured for 8 bit, LSB, no parity, and 1 stop bit. Reference AT+UART command.
  sercom4.initFrame(UART_CHAR_SIZE_8_BITS, LSB_FIRST, SERCOM_NO_PARITY, SERCOM_STOP_BIT_1);
  sercom4.initPads(PAD_TX, PAD_RX);
  // disable serial transmission FROM train TO controller
  SERCOM4->USART.CTRLB.bit.TXEN = 0;
  
  // END PMUXED DUPE OF UART begin()

  // Set up status pin
  pinMode(PIN_STATUS, INPUT);

  // Set up but do not enable watchdog until after connection established; watchdog will trip after ~30ms, or two missed messages.
  setUpWDT(); // enable WDT

  // Initial state
  writeLights(true, true, true, true);
  // Turn off motors
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  // Disable motors
  digitalWrite(enA, LOW);
  digitalWrite(enB, LOW);

  // hold for initial connection
  while(!(digitalRead(PIN_STATUS) == HIGH)) {
    static bool toggle = false;
    writeLights(toggle, toggle, toggle, toggle);
    toggle = !toggle;
    delay(100);
  }
  #ifdef TESTING
  Serial.println("Connection Established");
  #endif

  sercom4.enableUART(); // enable message reception on pins to HC-05
  enableWDT(); // Enable watchdog after connection established and message reception enabled
}

// no loop in ISR-reliant time-based FSM (for bluetooth demo purposes)
void loop() {}

/**
 * Turns on/off the headlights/brakelights
 *  a lights on engine a side; b lights on engine b side;
 */
void writeLights(bool aWhite, bool aRed, bool bWhite, bool bRed) {
  digitalWrite(aWhiteLEDPin, aWhite ? HIGH : LOW);
  digitalWrite(aRedLEDPin, aRed ? HIGH : LOW);
  digitalWrite(bWhiteLEDPin, bWhite ? HIGH : LOW);
  digitalWrite(bRedLEDPin, bRed ? HIGH : LOW);
}
