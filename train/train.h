
// Pins 4 and 5 use SERCOM4
const SERCOM BT_SERCOM = sercom4;
const uint32_t baudrate = 9600;

// Receiver Pin
const uint8_t PIN_RX = 5;
const EPortType PIN_RX_PORT = PORTB;
const uint8_t PIN_RX_PORT_PIN = 11;
const SercomRXPad PAD_RX = SERCOM_RX_PAD_3;

// Transmitter Pin
const uint8_t PIN_TX = 4;
const EPortType PIN_TX_PORT = PORTB;
const uint8_t PIN_TX_PORT_PIN = 10;
const SercomUartTXPad PAD_TX = UART_TX_PAD_2;

// Status Pin
const int PIN_STATUS = A6;
const int PIN_BT_POWER = A1;

// LED Pins
const int aWhiteLEDPin = 0;
const int aRedLEDPin = 1;
const int bWhiteLEDPin = 2;
const int bRedLEDPin = 3;

// raise minimum to least acceleration
const int MOTOR_MIN_OUT = 130;
// lower maximum to fastest desired speed
const int MOTOR_MAX_OUT = 255;

/**
 *  a lights on engine a side; b lights on engine b side;
 */
void writeLights(bool aWhite, bool aRed, bool bWhite, bool bRed);

/**
 * Serial data is encoded in atomic bytes. Each byte has a brake flag, direction flag, and speed value.
 * 
 * bits [0,5]: speed value from [0,63]
 * bit 6: direction flag (1 is forward)
 * bit 7: brake flag (1 is braked)
 *
 * Example    BRK   DIR   SPD   Expected Behavior
 * 01011001   FLS   FWD   25    Train moves forward at ~half speed with forward running lights.
 * 11000110   TRU   FWD   6     Train is stopped with brake lights.
 * 00111111   FLS   BCK   63    Train moves backward at full speed with backward running lights.
 * 01000000   FLS   FWD   0     Train is stopped with forward running lights.
 * 00000000   FLS   BCK   0     Train is stopped with backward running lights.
 */

inline uint8_t maskBrake(uint8_t b) {
    return b & 0x80;
}

inline uint8_t maskDirection(uint8_t b) {
    return b & 0x40;
}

inline uint8_t maskSpeed(uint8_t b) {
    return b & 0x3f;
}

int inline speedToDAC(int speed);
void inline writeMotorSpeed(int speed);
void setMotors(byte b);
