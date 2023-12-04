// Motor A connections
int enA = 9;
int in1 = 8;
int in2 = 7;
// Motor B connections
int enB = 3;
int in3 = 4;
int in4 = 5;
int speedValue;


void setUpMotors() {
	// Set all the motor control pins to outputs
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
}


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

