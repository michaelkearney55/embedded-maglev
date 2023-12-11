
void hardware_test() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(aWhiteLEDPin, HIGH);
    digitalWrite(aRedLEDPin, HIGH);
    digitalWrite(bWhiteLEDPin, HIGH);
    digitalWrite(bRedLEDPin, HIGH);
    digitalWrite(hb1Pin, HIGH);
    digitalWrite(hb2Pin, HIGH);
    analogWrite(motorPin, 255 * i / 10);
    delay(500);
    digitalWrite(aWhiteLEDPin, LOW);
    digitalWrite(aRedLEDPin, LOW);
    digitalWrite(bWhiteLEDPin, LOW);
    digitalWrite(bRedLEDPin, LOW);
    digitalWrite(hb1Pin, LOW);
    digitalWrite(hb2Pin, LOW);
    delay(500);
  }
}

void printStatus(byte b) {
  Serial.print("BRK: ");
  Serial.print(maskBrake(b) ? 'T' : 'F');
  Serial.print("\tDIR: ");
  Serial.print(maskDirection(b) ? 'T' : 'F');
  Serial.print("\tSPD: ");
  Serial.print((int) maskSpeed(b), DEC);
  Serial.print("\tMOT: ");
  Serial.print(analogRead(motor_testpin));
  Serial.print("\tHB1: ");
  Serial.print(digitalRead(hb1_testpin));
  Serial.print("\tHB2: ");
  Serial.print(digitalRead(hb2_testpin));
}