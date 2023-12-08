#include "utility.h"
#include "train_motor.h"
#include "watch_dog.h"

#define TESTING false

// 3 states: stop, foward, and backward
// enum State {
//   STOP = 1,
//   FORWARD = 2,
//   BACKWARD = 3,
// };

// Initial state will be set to STOP
State currentState = STOP;

// Global variables for train's status
int currentSpeed;
int speedReading;
int currentBrake;
int brakeReading;

// Interrupt button pin
const byte ISRPin = A2;

// ISR handler for when interrupts are triggered
void buttonISR() {
  if (digitalRead(ISRPin) == HIGH) {
    Serial.println("ISR triggered. Train stopping.");

    // stops the train and update the current state
    setMotorSpeed(0);
    currentState = STOP;
  }
}



/*        
 * Helper function for printing states
 */
char* s2str(State s) {
  switch(s) {
    case STOP:
    return "(1) STOP";
    case FORWARD:
    return "(2) FORWARD";
    case BACKWARD:
    return "(3) BACKWARD";
    default:
    return "???";
  }
}


void readInputs() {
  String inputString = Serial.readStringUntil('\n');

  int commaIndex = inputString.indexOf(',');
  String speedString = inputString.substring(0, commaIndex);
  String brakeString = inputString.substring(commaIndex + 1);

  speedReading = speedString.toInt();
  brakeReading = brakeString.toInt();
}

void printStatus() {
  // Print the current state name
  Serial.print("State: ");
  switch (currentState) {
    case STOP:
      Serial.print("STOP");
      break;
    case FORWARD:
      Serial.print("FORWARD");
      break;
    case BACKWARD:
      Serial.print("BACKWARD");
      break;
  }

  // Print the current speed and brake status
  Serial.print(", Current Speed: ");
  Serial.print(currentSpeed);
  Serial.print(", Brake: ");
  Serial.println(currentBrake ? "ON" : "OFF");
}


State updateFSM(State currentState, int speedReading, bool brakeReading) {
  // next state to return
  State nextState;

  if (speedReading >= 512) {
    speedReading = (speedReading - 512) / 2;
  }
  else {
    speedReading = -1 * (255 - (speedReading / 2));
  }

  switch (currentState) {
    case STOP:
      // the current speed reading is positive and brake is not being pressed
      // Transition 1 - 2
      if (speedReading > 0 && brakeReading == 0) {
        setMotorSpeed(speedReading);
        currentSpeed = speedReading;
        currentBrake = brakeReading;
        nextState = FORWARD;
      }

      // the current speed reading is negative and brake is not being pressed
      // Transition 1 - 3
      else if (speedReading < 0 && brakeReading == 0) {
        setMotorSpeed(speedReading);
        currentSpeed = speedReading;
        currentBrake = brakeReading;
        nextState = BACKWARD;
      }
      // the current speed reading is zero or brake is being pressed
      // Transition 1 - 1
      else {
        setMotorSpeed(0);
        currentSpeed = 0;
        currentBrake = brakeReading;
        nextState = STOP;
      }

      break;

    case FORWARD:
      // the brake is being pressed or the speedReading is set to zero
      // Trainsition 2 - 1
      if (brakeReading != 0 || speedReading == 0) {
        setMotorSpeed(0);
        currentSpeed = 0;
        currentBrake = brakeReading;
        nextState = STOP;
      }
      // the brake is not being pressed and the speedReading is positive
      // Trainsition 2 - 2
      else if (speedReading > 0) {
        setMotorSpeed(speedReading);
        currentSpeed = speedReading;
        currentBrake = brakeReading;
        nextState = FORWARD;
      }
      // the brake is not being pressed and the speedReading is negative
      // Trainsition 2 - 2
      else {
        setMotorSpeed(speedReading);
        currentSpeed = speedReading;
        currentBrake = brakeReading;
        nextState = BACKWARD;
      }
      
      break;

    case BACKWARD:
      // the brake is being pressed or the speedReading is set to zero
      // Trainsition 3 - 1
      if (brakeReading != 0 or speedReading == 0) {
        setMotorSpeed(0);
        currentSpeed = 0;
        currentBrake = brakeReading;
        nextState = STOP;
      }
      // the brake is not being pressed and the speedReading is positive
      // Trainsition 3 - 2
      else if (speedReading > 0) {
        setMotorSpeed(speedReading);
        currentSpeed = speedReading;
        currentBrake = brakeReading;
        nextState = FORWARD;
      }
      // the brake is not being pressed and the speedReading is negative
      // Trainsition 3 - 3
      else {
        setMotorSpeed(speedReading);
        currentSpeed = speedReading;
        currentBrake = brakeReading;
        nextState = BACKWARD;
      }
      
      break;
  }

  return nextState;
}



bool testTransition(State startState, State endState, int inputSpeed, bool inputBrake, int startSpeedVar, int endSpeedVar) {
	State resultState = updateFSM(startState, inputSpeed, inputBrake);

	bool passedTest = (endState == resultState && 
			endSpeedVar == currentSpeed);

	if (passedTest) {
    char sToPrint[200];
    sprintf(sToPrint, "Test from %s to %s PASSED", s2str(startState), s2str(endState));
		Serial.println(sToPrint);
		return true;
	}
	else {
    char sToPrint[200];
    Serial.println(s2str(startState));
    sprintf(sToPrint, "Test from %s to %s FAILED", s2str(startState), s2str(endState));

    sprintf(sToPrint, "Expected speed %d, got %d. resultState %s",endSpeedVar, currentSpeed, s2str(resultState));
    Serial.println(sToPrint);
    return false;
  }
}


void testAllTests() {

	for (int i = 0; i < numTests; i++) {
		Serial.print("Running test ");
		Serial.println(i);
		if (!testTransition(testStatesIn[i], testStatesOut[i], testInputSpeed[i], testInputBrake[i], testStartSpeedVar[i], testEndSpeedVar[i])) {
			return;
		}
	}
	Serial.println("All tests passed!");
}


void setup() {
  // set up interrupt button
  pinMode(ISRPin, INPUT_PULLUP);

  // set up the interrupt service
  attachInterrupt(digitalPinToInterrupt(ISRPin), buttonISR, CHANGE);

  // set up the pins for controlling the motors
  setUpMotors();

  // set up the watch dog timer
  setUpWDT();

  // Initialize serial communication
  Serial.begin(9600);  
}

  
void loop() {
  if (TESTING){
      testAllTests();
      delay(2000);
  }
  else if (Serial.available() > 0) {
    // Read the dummy inputs

    if (TESTING){
    		testAllTests();
    }
    else {
      readInputs();
    // Update the FSM
      currentState = updateFSM(currentState, speedReading, brakeReading);
    }

    // Print the current status
    printStatus();
  }

  // Pet the watch dog
  // Can remove this line to test the functionality of watch dog timer
  petWDT();
}

