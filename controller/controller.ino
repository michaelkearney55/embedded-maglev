#include "controller.h"

// Mode 0 = hardware
// Mode 1 = software
// #define TESTING
// #define TESTING_MODE 0
//define DEBUG


void setup() {

  #if defined(TESTING) || defined(DEBUG)
  Serial.begin(9600);
  while(!Serial);
  #endif

  // setup component inputs
  pinMode(knobPin, INPUT);
  pinMode(brakePin, INPUT);
  pinMode(allclearPin, INPUT);
  pinMode(btStatusPin, INPUT);

  // configure LED output pins
  pinMode(fPin, OUTPUT);
  pinMode(bPin, OUTPUT);
  pinMode(brakeLEDPin, OUTPUT);

  // configure eight segment pins
  for(int i = 0; i < 8; i++) {
    pinMode(eightSegmentPins[i], OUTPUT);
  }

  #ifdef DEBUG
  Serial.print("Set pin modes.\n");
  #endif

  // trigger button flags on button press
  attachInterrupt(brakePin, flagBrake, RISING);
  attachInterrupt(allclearPin, flagAllclear, RISING);

  #ifdef DEBUG
  Serial.print("Attached interrupts to button pins.\n");
  #endif

  // read ADC at lower bit resolution to reduce chatter
  analogReadResolution(readResolution);

  #ifdef DEBUG
  Serial.print("Set readResolution to ");
  Serial.print(readResolution);
  Serial.print(".\n");
  #endif

  Serial1.begin(9600);

  writeLEDs(false, false, false);
  display(connectingCode);
  
}

void loop() {
  #ifdef DEBUG
  int myDelay = 2000;
  #else
  int myDelay = 10;
  #endif

  static long lastUpdate = 0;
  #ifdef TESTING
  #if TESTING_MODE == 0 // hardware testing
  Serial.print("Hardware testing mode activated.\n");
  testEightSegment();
  updateInputs();
  printInputs();
  delay(500);
  #elif TESTING_MODE == 1 //
  Serial.print("Software testing mode activated.\n");
  Serial.print("\nTesting knobToDisplaySpeed function\n");
  for (int i = 0; i < (1 << readResolution); i++) {
    Serial.print(i);
    Serial.print(" -> ");
    Serial.print(knobToDisplaySpeed(i));
    Serial.print('\n');
  }
  Serial.print("\nTesting knobToMessage function\n");
  for (int i = 0; i < (1 << readResolution); i++) {
    Serial.print(i);
    Serial.print(" -> ");
    Serial.print(knobToMessage(i));
    Serial.print('\n');
  }
  while(1);
  #endif
  #else
  int now = millis();
  if (now - lastUpdate > myDelay){
    lastUpdate = now;
    static state CURRENT_STATE = INIT_STATE;
    updateInputs();
    CURRENT_STATE = updateFSM(CURRENT_STATE);
  }
  #endif

}

/**
 * INPUTS
 * static bool connected; // if bluetooth module is connected to another module
 * static bool brake; // if the brake is active
 * static int knob; // current reading of the speed knob
 */
state updateFSM(state curState) {
  static bool lastDirIsForward = true;
  #ifdef DEBUG
  Serial.print("lastDirIsForward: ");
  Serial.print(lastDirIsForward);
  Serial.print("; curState: ");
  Serial.println(curState);
  #endif
  state nextState;
  switch(curState) {
    case CONNECT:
      if (connected) {
        if (brake) {
          writeLEDs(false, false, true);
          display(brakeCode);
          sendMessage(lastDirIsForward ? brakeMessageMask | dirMessageMask : brakeMessageMask);
          nextState = BRAKE;
        } else {
          byte message = knobToMessage(knob);
          if (message) { // speed is not zero
            lastDirIsForward = message & dirMessageMask;
          } else if (lastDirIsForward) { // add dir flag if message has zero speed
            message |= dirMessageMask;
          }
          writeLEDs(lastDirIsForward, !lastDirIsForward, false);
          display(digitCodes[knobToDisplaySpeed(knob)]);
          sendMessage(message);
          nextState = MOVE;
        }
      } else {
        static bool flash = true;
        if (flash) { // flash all LEDs while connecting
          writeLEDs(false, false, false);
        } else {
          writeLEDs(true, true, true);
        }
        flash = !flash;
        nextState = CONNECT;
      }
      break;
    case BRAKE:
      if (connected) {
        if (brake) {
          sendMessage(lastDirIsForward ? brakeMessageMask | dirMessageMask : brakeMessageMask);
          nextState = BRAKE;
        } else {
          byte message = knobToMessage(knob);
          if (message) { // speed is not zero
            lastDirIsForward = message & dirMessageMask;
          } else if (lastDirIsForward) { // add dir flag if message has zero speed
            message |= dirMessageMask;
          }
          writeLEDs(lastDirIsForward, !lastDirIsForward, false);
          display(digitCodes[knobToDisplaySpeed(knob)]);
          sendMessage(message);
          nextState = MOVE;
        }
      } else {
        if (millis() & 0x200) { // flash all LEDs every second while connecting
          writeLEDs(false, false, false);
        } else {
          writeLEDs(true, true, true);
        }
        display(connectingCode);
        nextState = CONNECT;
      }
      break;
    case MOVE:
      if (connected) {
        if (brake) {
          writeLEDs(false, false, true);
          display(brakeCode);
          sendMessage(lastDirIsForward ? brakeMessageMask | dirMessageMask : brakeMessageMask);
          nextState = BRAKE;
        } else {
          byte message = knobToMessage(knob);
          if (message) { // speed is not zero
            lastDirIsForward = message & dirMessageMask;
          } else if (lastDirIsForward) { // add dir flag if message has zero speed
            message |= dirMessageMask;
          }
          writeLEDs(lastDirIsForward, !lastDirIsForward, false);
          display(digitCodes[knobToDisplaySpeed(knob)]);
          sendMessage(message);
          nextState = MOVE;
        }
      } else {
        writeLEDs(false, false, false);
        display(connectingCode);
        nextState = CONNECT;
      }
      break;
    default:
    nextState = curState;
  }
  #ifdef DEBUG
  Serial.print("nextState: ");
  Serial.println(nextState);
  #endif
  return nextState;
}

void updateInputs() {
  // atomically read interrupt-driven button flags to input vars
  noInterrupts();
  brake = brakeFlag;
  interrupts();

  connected = digitalRead(btStatusPin);

  knob = analogRead(knobPin);
}

/**
 *  flagBrake is an ISR that turns on the brakeFlag
 */
void flagBrake() {
  brakeFlag = true;
  #if TESTING_MODE == 0 || defined(DEBUG)
  Serial.println("Brake pressed.");
  #endif
}

/**
 *  flagAllclear is an ISR that turns on the allclearFlag
 */
void flagAllclear() {
  brakeFlag = false;
  #if TESTING_MODE == 0 || defined(DEBUG)
  Serial.println("allclear pressed.");
  #endif
}

int inline knobToDisplaySpeed(int reading) {
  int ret = ((reading + (1 << (readResolution - 6))) / (6 * (1 << (readResolution - 6)))) - 5;
  #ifdef DEBUG
  Serial.print("knobToDisplaySpeed(");
  Serial.print(reading);
  Serial.print("): ");
  Serial.println(ret);
  #endif
  return abs(ret); 
}

byte inline knobToMessage(int reading) {
  byte ret;
  if (reading < zeroLeftBound) { // negative
    ret = (byte) map(reading, backwardLeftBound,  backwardRightBound, 63, 0);
  } else if (reading < forwardLeftBound) { // zero
    ret = 0;
  } else { // forward include direction bit
    ret = 0x40 | (byte) map(reading, forwardLeftBound, forwardRightBound, 0, 63);
  }
  #ifdef DEBUG
  Serial.print("knobToMessage(");
  Serial.print(reading);
  Serial.print("): ");
  Serial.println(ret);
  #endif
  return ret;
}

bool inline knobIsForward(int reading) {
  bool ret = reading > (29 * (1 << (readResolution - 6)));
  #ifdef DEBUG
  Serial.print("knobIsForward(");
  Serial.print(reading);
  Serial.print("): ");
  Serial.println(ret);
  #endif
  return ret;
}

void inline writeLEDs(bool forward, bool backward, bool brake) {
  digitalWrite(fPin, forward ? HIGH : LOW);
  digitalWrite(bPin, backward ? HIGH : LOW);
  digitalWrite(brakeLEDPin, brake ? HIGH : LOW);
}

void sendMessage(byte b) {
  #ifdef DEBUG
  Serial.print("Sending message ");
  Serial.println((int) b, BIN);
  #endif
  Serial1.write(b);
}

void display(byte b) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(eightSegmentPins[i], (b & 1) ? HIGH : LOW);
    b = b >> 1;
  }
}