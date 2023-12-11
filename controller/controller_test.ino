void printInputs() {
  Serial.print("Knob: ");
  Serial.print(knob);
  Serial.print(";\tBrake: ");
  Serial.print(brake);
  Serial.print(";\tConnected: ");
  Serial.print(connected);
  Serial.print(";\n");
}

void printOutputs() {
  Serial.print("Knob: ");
  Serial.print(knob);
  Serial.print(";\tBrake: ");
  Serial.print(brake);
  Serial.print(";\tConnected: ");
  Serial.print(connected);
  Serial.print(";\n");
}

void testEightSegment() {
  for (int i = 0; i < 10; i++) {
    display(digitCodes[i]);
    delay(100);
  }
  display(connectingCode);
  delay(100);
  display(brakeCode);
  delay(100);
  display(errorCode);
  delay(100);
  display(emptyCode);
}