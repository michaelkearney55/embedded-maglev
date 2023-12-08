typedef enum {
  STOP = 1,
  FORWARD = 2,
  BACKWARD = 3,
} State;

int numTests = 8;

const State testStatesIn[8] = {(State) 1, (State) 1, (State) 1, (State) 2, (State) 2, (State) 2, (State) 3, (State) 3};
const State testStatesOut[8] = {(State) 1, (State) 2, (State) 3, (State) 1, (State) 1, (State) 2, (State) 1, (State) 3};
const int testInputSpeed[8] = {-1, 1023, 0, 512, -1, 1000, -1, 10};
const bool testInputBrake[8] = {true, false, false, false, true, false, true, false};
const int testStartSpeedVar[8] = {0, 0, 0, 255, -1, 255, -1, 0};
const int testEndSpeedVar[8] = {0, 255, -255, 0, 0, 244, 0, -250};
