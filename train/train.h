
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
const int PIN_STATUS = 3;

// LED Pins
// Lights were implemented for visual feedback to avoid wasting battery power in testing
const int aWhiteLEDPin = A1; // side A headlights
const int aRedLEDPin = A2; // side A brakelight
const int bWhiteLEDPin = A3; // side B headlights
const int bRedLEDPin = A4; // side B brakelight

// en pins enable appropriate motor, in pins select direction when configured
// Motor A connections
int enA = 7;
int in1 = 8;
int in2 = 9;
// Motor B connections
int enB = 12;
int in3 = 10;
int in4 = 11;

// calibrated min/max PWM vals for motor acceleration
const int MOTOR_MIN_OUT = 130;
// lower maximum to fastest desired speed
const int MOTOR_MAX_OUT = 255;




/**
 * Serial data is encoded in atomic bytes. Each byte has a brake flag, direction flag, and speed value.
 * 
 * bits [0,5]: speed value from [0,63]
 * bit 6: direction flag (1 is forward, 0 is backward)
 * bit 7: brake flag (1 is braked, 0 is not braked)
 *
 * Example    BRK   DIR   SPD   Message meaning
 * 01011001   FLS   FWD   25    Train moves forward at ~half speed with forward running lights.
 * 11000110   TRU   FWD   6     Train is braked; stopped with brake lights.
 * 00111111   FLS   BCK   63    Train moves backward at full speed with backward running lights.
 * 01000000   FLS   FWD   0     Train is stopped with forward running lights.
 * 00000000   FLS   BCK   0     Train is stopped with backward running lights.
 */

// returns brake flag of message
inline uint8_t maskBrake(uint8_t b) {
    return b & 0x80;
}

// returns direction flag of message
inline uint8_t maskDirection(uint8_t b) {
    return b & 0x40;
}

// returns speed value of message
inline uint8_t maskSpeed(uint8_t b) {
    return b & 0x3f;
}

void writeLights(bool aWhite, bool aRed, bool bWhite, bool bRed);


/**
 * Maps a speed [0,63] to a direction independent DAC output as follows:
 * 0 = 0
 * 1 = MOTOR_MIN_OUT
 * 63 = MOTOR_MAX_OUT;
 * 
 * This is implemented as 
*/
int inline speedToDAC(int speed) {
    if (speed) {
        speed = map(speed, 1, 63, MOTOR_MIN_OUT, MOTOR_MAX_OUT);
    }
    return speed;
}