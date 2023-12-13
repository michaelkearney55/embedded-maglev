
void hardware_test() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(aWhiteLEDPin, HIGH);
    digitalWrite(aRedLEDPin, HIGH);
    digitalWrite(bWhiteLEDPin, HIGH);
    digitalWrite(bRedLEDPin, HIGH);
    delay(500);
    digitalWrite(aWhiteLEDPin, LOW);
    digitalWrite(aRedLEDPin, LOW);
    digitalWrite(bWhiteLEDPin, LOW);
    digitalWrite(bRedLEDPin, LOW);
    delay(500);
  }
}

void printStatus(byte b) {
  #ifdef TESTING
  Serial.print("BRK: ");
  Serial.print(maskBrake(b) ? 'T' : 'F');
  Serial.print("\tDIR: ");
  Serial.print(maskDirection(b) ? 'T' : 'F');
  Serial.print("\tSPD: ");
  Serial.print((int) maskSpeed(b), DEC);
  #endif
}