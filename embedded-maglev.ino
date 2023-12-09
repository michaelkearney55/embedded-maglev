#include <SPI.h>
#include <WiFi101.h>
#include "arduino_secrets.h" 
#include "utility.h"
#include "wifi-status.h" 
#include "fsm.h" 

State test;

#define MOCK_WIFI true
#define IS_SERVER false
#define TESTING false

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 1L * 1000L; // delay between updates, in milliseconds

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
int status = WL_IDLE_STATUS;     // the WiFi radio's status
int potentiometer = 10;

WiFiServer server(80);
WiFiClient client;
IPAddress a_server(172,18,135,234);


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  // wait for serial port to connect. Needed for native USB port only
  while (!Serial) {;}


  // set up interrupt button
  pinMode(ISRPin, INPUT_PULLUP);

  // set up the interrupt service
  attachInterrupt(digitalPinToInterrupt(ISRPin), buttonISR, CHANGE);

  // set up the pins for controlling the motors
  setUpMotors();

  // set up the watch dog timer
  setUpWDT();

  if (!MOCK_WIFI) {
    connectToWifi(ssid, &status);

    if (IS_SERVER) {
      server.begin();
    }
  }
  else {
    Serial.println("Mock wifi connection established, use serial monitor to input values");
  }
}


// this method makes a HTTP connection to the server:
void sendHttpRequest() {

  // client.stop();
  // if there's a successful connection:
  // Serial.println("attempting to connect");
  if (client.connect(a_server, 80)) {

    Serial.println("connection completed");
    potentiometer += 1; // TODO log potentiometer values
    client.println(String(potentiometer)); // send potentiometer values to server
    lastConnectionTime = millis();
  }
  else { // if you couldn't make a connection:
    // Serial.println("connection failed");
  }
}

void loop() {
  // check the network connection once every 10 seconds:
  // delay(5000);
  // printCurrentNet();

  if (TESTING){
      testAllTests();
      delay(2000);
  }
  else if (MOCK_WIFI && Serial.available() > 0) {
    // Read the dummy inputs
    String inputString = Serial.readStringUntil('\n');
    readInputs(inputString);
    // Update the FSM
    currentState = updateFSM(currentState, speedReading, brakeReading);
    // Print the current status
    printStatus();
  }
  else {
    if (IS_SERVER) {
      WiFiClient a_client = server.available();
      if (a_client) {
        Serial.println("new client");
        while (a_client.connected()) {
          if (a_client.available()) {
            String request = a_client.readStringUntil('\n');
            readInputs(request); // RECEIVED POTENTIOMETER VALUE, update for FSM
            currentState = updateFSM(currentState, speedReading, brakeReading);
            printStatus();
          }
        }
      }
    }
    else { // is client
      if (millis() - lastConnectionTime > postingInterval) {
        sendHttpRequest();
      }
    }
    delay(1);
  }

  // Can remove this line to test the functionality of watch dog timer
  petWDT();  // Pet the watch dog
}
