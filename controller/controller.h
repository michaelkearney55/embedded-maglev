// pin definitions
const int knobPin = A1;
const int btStatusPin = 12;
const int brakePin = 7;
const int allclearPin = 8;
const int brakeLEDPin = 9;
const int fPin = A2;
const int bPin = A3;
const int eightSegmentPins[] = {A5, A6, 0, 1, 2, 3, 4, 5};

// inputs
static bool connected; // if bluetooth module is connected to another module
static bool brake; // if the brake is active
static int knob; // current reading of the speed knob

// button flags are toggled by interrupts from buttons
// button flags atomically update brake input and allclear variable
static volatile bool brakeFlag;
static volatile bool allclearFlag;

typedef enum {
  CONNECT = 1,
  MOVE = 2,
  BRAKE = 3,
} state;

const state INIT_STATE = CONNECT;

/** 8 SEGMENT DISPLAY ENCODINGS
 *  Each segment of the display is controlled by a different bit [0-6] according to the diagram. Bit 7 is reserved for the direction LED.
 * 
 *      / -- 2 -- /
 *     1         3
 *    / -- 0 -- /
 *   4         6
 *  / -- 5 -- /
 *
 *  digitMap encodes digits [0,9] at the respective index.
 *  brakeCode encodes 'H'
 *  connectingCode encodes 'C'
 *  errorCode encodes 'E'
 */
const byte brakeCode = B01011011; // 'H'
const byte connectingCode = B00110110; // 'C'
const byte emptyCode = B00000000; // off
const byte errorCode = B00110111; // 'E'
const byte digitCodes[] = {B01111110, B01001000, B00111101, B01101101, B01001011, B01100111, B01110111, B01001100, B01111111, B01101111};

/** MESSAGE ENCODINGS
 *  Messages are one byte long. Bits [0,6] encode speed.
 *  Bit 7 is a brake flag and should be prioritized over any speed value.
 */
const byte brakeMessageMask = 0x80;
const byte dirMessageMask = 0x40;

/**
 * knobToSpeed maps a binary-aligned value with of least 6 bits into a signed speed [-5,5] without fractional math through bucketing
 * 
 * 6-bit example:
 * speed value: -5 -4 -3 -2 -1  0  1  2  3  4  5
 * bucket size: [5][6][6][6][6][6][6][6][6][6][5] (64 total bits)

 * 7-bit example:
 * speed value:  -5  -4  -3  -2  -1   0   1   2   3   4   5
 * bucket size: [10][12][12][12][12][12][12][12][12][12][10] (128 total bits)
 *
 * readResolution is n_bits read by ADC. readResolution must be at least 6
 */
const int readResolution = 6;
const int backwardLeftBound = 0;
const int zeroLeftBound = 29 * pow(2, (readResolution - 6));
const int forwardLeftBound = 35 * pow(2, (readResolution - 6));
const int backwardRightBound = zeroLeftBound - 1;
const int zeroRightBound = forwardLeftBound - 1;
const int forwardRightBound = 64 * pow(2, (readResolution - 6)) - 1;


state updateFSM(state curState);
void updateInputs();
void flagBrake();
void flagAllclear();
int inline knobToDisplaySpeed(int reading);
int inline knobToBitSpeed(int reading, int n_bits);
byte inline knobToMessage(int reading);
bool inline knobIsForward(int reading);
void inline writeLEDs(bool forward, bool backward, bool brake);
void sendMessage(byte b);
void display(byte b);