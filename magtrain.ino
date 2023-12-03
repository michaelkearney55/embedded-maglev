#include <SPI.h>
#include <WiFi101.h>
#include "arduino_secrets.h" 
#include "wifi-status.h" 
// #include <WiFi.h>


#define IS_SERVER true

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
int status = WL_IDLE_STATUS;     // the WiFi radio's status

WiFiServer server(80);
WiFiClient client;

char a_server[] = "";

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to open SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWiFiData();

  Serial.print("Connected to wifi. My address:");
  IPAddress myAddress = WiFi.localIP();
  Serial.println(myAddress);
  
  if (IS_SERVER) {
    server.begin();
  }
  else {

  }
}
void loop() {
  // check the network connection once every 10 seconds:
  // delay(5000);
  // printCurrentNet();

  delay(500);
  if (IS_SERVER) {
    WiFiClient a_client = server.available();
    if (a_client && a_client.connected()) {
      String request = a_client.readStringUntil('\n');
      Serial.println(request);
      a_client.print("response\n");
      a_client.stop();
    }
  }
  else {
          // Make a HTTP request:

    if (client.connect(a_server, 80)) {
      Serial.println("connected");
      client.println("hello");
    }
  }

}
