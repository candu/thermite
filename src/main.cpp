#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "private.h"

#define ESP8266_LED 5

WiFiServer server(80);

void initPins() {
  pinMode(ESP8266_LED, OUTPUT);
  digitalWrite(ESP8266_LED, LOW);
}

void initSerial() {
  Serial.begin(115200);
  Serial.println();
}

uint8_t initWifiConnection() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PW);

  Serial.print("Connecting");
  int i = 0;
  while (true) {
    // check wifi status
    uint8_t status = WiFi.status();
    if (status == WL_NO_SSID_AVAIL
      || status == WL_CONNECTED
      || status == WL_CONNECT_FAILED) {
      return status;
    }

    // blink LED, print to serial
    int led = i % 2 == 0 ? HIGH : LOW;
    digitalWrite(ESP8266_LED, led);
    delay(100);
    if (i % 5 == 0) {
      Serial.print(".");
    }
    i++;
  }
}

void notifyWifiStatus(uint8_t status) {
  if (status == WL_NO_SSID_AVAIL) {
    Serial.print("Invalid SSID!");
  } else if (status == WL_CONNECTED) {
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
  } else if (status == WL_CONNECT_FAILED) {
    Serial.println("Invalid password!");
  }

  digitalWrite(ESP8266_LED, LOW);
  Serial.println();
}

void setup() {
  initPins();
  initSerial();

  uint8_t status = initWifiConnection();
  notifyWifiStatus(status);
  server.begin();
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Prepare the response. Start with the common header:
  String s =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<!DOCTYPE HTML>\n<html>\n"
    "<body><h1>It works!</h1></body></html>\n";

  // Send the response to the client
  client.print(s);
}
