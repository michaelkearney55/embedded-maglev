#include "train_motor.h"

// 3 states: stop, foward, and backward
enum State {
  STOP,
  FORWARD,
  BACKWARD,
};

// Initial state will be set to STOP
State currentState = STOP;

// Global variables for train's status
int currentSpeed;
int speedReading;
int currentBrake;
int brakeReading;

void setup() {
  setUpMotors();
  Serial.begin(9600);  // Initialize serial communication
}

  
void loop() {
  if (Serial.available() > 0) {
    // Read the dummy inputs
    readInputs();

    // Update the FSM
    currentState = updateFSM(currentState, speedReading, brakeReading);

    // Print the current status
    printStatus();
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

