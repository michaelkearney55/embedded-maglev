#include "train.h"
#include "watch_dog.h"
#include "motor.h"

#define TESTING

static volatile int counter = 0;

// DO NOT call begin() on Serial4. We must configure the PMUX registers manually.
Uart Serial4(&sercom4, PIN_RX, PIN_TX, PAD_RX, PAD_TX);


// OVERWRITE SERCOM ISR TO HANDLE UART, HANDLE STATE CHANGE, AND PET WATCHDOG
void SERCOM4_Handler() {
  if (SERCOM4->USART.INTFLAG.bit.RXC) {
    if (SERCOM4->USART.INTFLAG.bit.ERROR) {
      // IGNORE THE MESSAGE
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
      Serial.print(speedToDAC((int) maskSpeed(b)));
      Serial.print('\n');
      #endif

      // We received a message. Handle the message
      if (maskBrake(b)) {
        analogWrite(enA, 0);
        analogWrite(enB, 0);
        writeLights(false, true, false, true); // red lights
      } else {
        //writeHBridgeDirection(maskDirection(b));
        if (maskDirection(b)) {
          digitalWrite(in1, HIGH);
          digitalWrite(in2, LOW);
          digitalWrite(in3, HIGH);
          digitalWrite(in4, LOW);
          writeLights(true, false, false, true); // forward lights
        } else {
          digitalWrite(in1, LOW);
          digitalWrite(in2, HIGH);
          digitalWrite(in3, LOW);
          digitalWrite(in4, HIGH);
          writeLights(false, true, true, false); // forward lights
        }
        int speed = speedToDAC((int) maskSpeed(b));
        analogWrite(enA, speed);
        analogWrite(enB, speed);
      }

      // Write the speed value to the DAC motor controller
      // Even if brake is enabled, write to DAC so we're ready when brake disabled
      // PET WATCHDOG
      // WDT->CLEAR.reg = 0xA5;

      // clear interrupt register
      SERCOM4->USART.INTFLAG.bit.RXC = 1;
    }
  }

  // discard error interrupts, as implemented in Arduino library
  if (SERCOM4->USART.INTFLAG.bit.ERROR || SERCOM4->USART.INTFLAG.bit.DRE) {
      SERCOM4->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
  }
}

void setup() {


  Serial.begin(9600);
  while(!Serial);

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
  // pinMode(motorPin, OUTPUT);
  // pinMode(hb1Pin, OUTPUT);
  // pinMode(hb2Pin, OUTPUT);
  setUpMotors();

  // Turn off motors - Initial state
  //output((byte) B10000000);

  // TODO Set up HC-05 Serial communication


  // BEGIN PMUXED DUPE OF UART begin()
  
  // Configure PIN_TX (5, PB11, SERCOM4, PAD2) as output in PORT as SERCOM-ALT with PMUX function D
  uint8_t temp = PORT->Group[PIN_TX_PORT].PMUX[PIN_TX_PORT_PIN >> 1].reg & (PIN_TX_PORT_PIN & 0x1 ? PORT_PMUX_PMUXE_Msk : PORT_PMUX_PMUXO_Msk);
  PORT->Group[PIN_TX_PORT].PMUX[PIN_TX_PORT_PIN >> 1].reg = temp | (PIN_TX_PORT_PIN & 0x1 ? PORT_PMUX_PMUXO_D : PORT_PMUX_PMUXE_D);
  PORT->Group[PIN_TX_PORT].PINCFG[PIN_TX_PORT_PIN].reg |= PORT_PINCFG_PMUXEN;

  // Configure PIN_RX (4, PB10, SERCOM4, PAD3) as input in PORT as SERCOM-ALT with PMUX function D
  
  temp = PORT->Group[PIN_RX_PORT].PMUX[PIN_RX_PORT_PIN >> 1].reg & (PIN_RX_PORT_PIN & 0x1 ? PORT_PMUX_PMUXE_Msk : PORT_PMUX_PMUXO_Msk);
  PORT->Group[PIN_RX_PORT].PMUX[PIN_RX_PORT_PIN >> 1].reg = temp | (PIN_RX_PORT_PIN & 0x1 ? PORT_PMUX_PMUXO_D : PORT_PMUX_PMUXE_D);
  PORT->Group[PIN_RX_PORT].PINCFG[PIN_RX_PORT_PIN].reg |= PORT_PINCFG_PMUXEN;

  sercom4.initUART(UART_INT_CLOCK, SAMPLE_RATE_x16, baudrate);
  sercom4.initFrame(UART_CHAR_SIZE_8_BITS, LSB_FIRST, SERCOM_NO_PARITY, SERCOM_STOP_BIT_1);
  sercom4.initPads(PAD_TX, PAD_RX);
  SERCOM4->USART.CTRLB.bit.TXEN = 0;
  
  // END PMUXED DUPE OF UART begin()

  // Set up status pin
  pinMode(PIN_STATUS, INPUT);

  // Initial state

  writeLights(true, true, true, true);
  writeHBridge(false, false); // turn off motors

  // hold for initial connection
  while(!(digitalRead(PIN_STATUS) == HIGH)) {
    static bool toggle = false;
    writeLights(toggle, toggle, toggle, toggle);
    toggle = !toggle;
    delay(100);
  }
  Serial.println("Connection Established");

  // TODO: add intial longer period
  setUpWDT(); // enable WDT
  sercom4.enableUART(); // enable connection
  enableWDT();
}

void loop() {
  // static long then = 0;
  // long now = millis();
  // if (now - then > 1000) {
  //   int myCounter;
  //   noInterrupts();
  //   myCounter = counter;
  //   counter = 0;
  //   interrupts();
  //   Serial.print(myCounter);
  //   Serial.print(" in ");
  //   Serial.print(now-then);
  //   Serial.print(" ms\n");
  //   then = now;
  // }
}

void writeLights(bool aWhite, bool aRed, bool bWhite, bool bRed) {
  digitalWrite(aWhiteLEDPin, aWhite ? HIGH : LOW);
  digitalWrite(aRedLEDPin, aRed ? HIGH : LOW);
  digitalWrite(bWhiteLEDPin, bWhite ? HIGH : LOW);
  digitalWrite(bRedLEDPin, bRed ? HIGH : LOW);
}

/**
 * Maps a speed [0,63] to a direction independent DAC output as follows:
 * 0 = 0
 * 1 = DAC_MIN_OUT
 * 63 = DAC_MAX_OUT;
 * 
 * This preserves a 0 speed as no DAC power and enables a minimum DAC output level for the lowest speed.
*/
int inline speedToDAC(int speed) {
    if (speed) {
        speed = map(speed, 1, 63, MOTOR_MIN_OUT, MOTOR_MAX_OUT);
    }
    return speed;
}

void inline writeHBridge(bool p1, bool p2) {
    digitalWrite(hb1Pin, p1 ? HIGH : LOW);
    digitalWrite(hb2Pin, p2 ? HIGH : LOW);
}

void inline writeHBridgeOff() {
    writeHBridge(false, false);
}

void inline writeHBridgeDirection(bool forward) {
    writeHBridge(forward, !forward);
}

/**
 * Sets the motors according to an encoded byte
*/
void inline setMotors(byte b) {
    // if not braked, enables and sets direction of h bridge
    if (maskBrake(b)) {
        writeHBridgeOff();
    } else {
        writeHBridgeDirection(maskDirection(b));
    }

    // Write the speed value to the DAC motor controller
    // Even if brake is enabled, write to DAC so we're ready when brake disabled
    analogWrite(motorPin, speedToDAC((int) maskSpeed(b)));
}