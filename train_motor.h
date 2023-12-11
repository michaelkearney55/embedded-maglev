// Motor A connections
int enA = 9;
int in1 = 8;
int in2 = 7;
// Motor B connections
int enB = 3;
int in3 = 4;
int in4 = 5;
int speedValue;

/**
 * Configures motor pins as outputs and initializes them as OFF.
*/
void setUpMotors() {
    // Set all the motor control pins to outputs
    pinMode(enA, OUTPUT);
    pinMode(enB, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);

    // Turn off motors - Initial state
    writeHBridge(false, false, false, false);
}

const int MOTOR_MIN_OUT = 130;
const int MOTOR_MAX_OUT = 255;

byte inline maskBrake(byte b) {
    return b & 0x80;
}

byte inline maskDirection(byte b) {
    return b & 0x40;
}

bool inline maskVal(byte b) {
    return b & 0x3f;
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

void inline writeHBridge(bool p1, bool p2, bool p3, bool p4) {
    digitalWrite(in1, p1 ? HIGH : LOW);
    digitalWrite(in2, p2 ? HIGH : LOW);
    digitalWrite(in3, p3 ? HIGH : LOW);
    digitalWrite(in4, p4 ? HIGH : LOW);
}

void inline writeHBridgeOff() {
    writeHBridge(false, false, false, false);
}

void inline writeHBridgeDirection(bool forward) {
    writeHBridge(forward, !forward, forward, !forward);
}

void inline writeMotorSpeed(int speed) {
    analogWrite(enA, speed);
    analogWrite(enB, speed);
}

/**
 * Sets the motors according to an encoded byte
*/
void setMotors(byte b) {
    // if not braked, enables and sets direction of h bridge
    if (maskBrake(b)) {
        writeHBridgeOff();
    } else {
        writeHBridgeDirection(maskDirection(b));
    }

    // Write the speed value to the DAC motor controller
    // Even if brake is enabled, write to DAC so we're ready when brake disabled
    writeMotorSpeed(speedToDAC((int) maskVal(b)));
}

/**
 * 
*/
void setMotorSpeed(int speed) {
    // if the speed is set to zero, motor stays still
    if (speed == 0) {
      analogWrite(enA, 0);
      analogWrite(enB, 0);
      return;
    }

    // Ensure the absolute value of the speed is at least 130 or -130, but not more than 255 or less than -255
    // to generate enough power to move the motor
    if (abs(speed) < 130) {
        speed = (speed < 0) ? -130 : 130;
    }

    // Set the direction based on the sign of the speed
    if (speed > 0) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
    } 
    
    else {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
        speed = -speed; // Make speed positive for analogWrite
    }

    // Write the absolute speed value to the motor controller
    analogWrite(enA, speed);
    analogWrite(enB, speed);
}

