/*
  IR Breakbeam sensor demo!
*/
#include <Ticker.h>
#include <ESP8266WiFi.h>
// Pin 13: Arduino has an LED connected on pin 13
// Pin 11: Teensy 2.0 has the LED on pin 11
// Pin  6: Teensy++ 2.0 has the LED on pin 6
// Pin 13: Teensy 3.0 has the LED on pin 13

#define SENSORPIN D4
//const char* server = "184.106.153.149";
const char* ssid     = "";
const char* password = "";
const int DELAY_SECONDS = 300;

IPAddress server(184, 106, 153, 149);
String apiKey = "";

// variables will change:
int sensorState = 0, lastState = 0, breakCount = 0;     // variable for reading the pushbutton status
Ticker ticker;
long startMillis = 0;
bool isSending = false;

void setup() {
  Serial.begin(115200);
  // initialize the LED pin as an output:
  pinMode(BUILTIN_LED, OUTPUT);
  // initialize the sensor pin as an input:
  pinMode(SENSORPIN, INPUT);
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // ticker.attach(DELAY_SECONDS, sendToTS);
  startMillis = millis();
}

void loop() {
  // read the slntate of the pushbutton value:
  sensorState = digitalRead(SENSORPIN);
  // check if the sensor beam is broken
  // if it is, the sensorState is LOW:
  if (sensorState == LOW) {
    // turn LED on:
    digitalWrite(BUILTIN_LED, HIGH);
  }
  else {
    // turn LED off:
    digitalWrite(BUILTIN_LED, LOW);
  }

  if (sensorState && !lastState) {
    Serial.println("Unbroken");
  }
  if (!sensorState && lastState) {
    Serial.println("Broken");
    breakCount ++;
  }
  lastState = sensorState;
  if (millis() - startMillis > DELAY_SECONDS * 1000) {
    sendToTS();
    startMillis = millis();
  }
}

void sendToTS() {
  isSending = true;
  delay(100);
  yield();
  WiFiClient client;

  if (client.connect(server, 80)) { // use ip 184.106.153.149 or api.thingspeak.com
    Serial.println("WiFi Client connected ");

    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(breakCount);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    breakCount = 0;
    delay(100);
  } else {
    Serial.println("Couldn't connect....");
  }
  client.stop();
  isSending = false;
}

