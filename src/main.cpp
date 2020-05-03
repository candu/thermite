#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

#include "private.h"

#define ESP8266_LED 5

AsyncWebServer server(80);

// HARDWARE SETUP

void initHardware() {
  pinMode(ESP8266_LED, OUTPUT);
  digitalWrite(ESP8266_LED, LOW);

  Serial.begin(115200);
  Serial.println();
}

// WIFI CONNECTION SETUP

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

uint8_t initWifi() {
  uint8_t status = initWifiConnection();
  notifyWifiStatus(status);
  return status;
}

// WEB SERVER SETUP

enum HttpStatusCode {
  HTTP_OK = 200,
  HTTP_BAD_REQUEST = 400,
  HTTP_NOT_FOUND = 404
};

struct HttpError {
  HttpStatusCode code;
  String message;
};

int led = LOW;

void sendJsonError(AsyncWebServerRequest* request, const HttpError& error) {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  response->setCode(error.code);

  const JsonObject& root = response->getRoot();
  root["code"] = error.code;
  root["message"] = error.message.c_str();

  response->setLength();
  request->send(response);
}

void sendJsonSettings(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse();

  const JsonObject& root = response->getRoot();
  root["led"] = led;

  response->setLength();
  request->send(response);
}

void notFound(AsyncWebServerRequest* request) {
  HttpError error { HTTP_NOT_FOUND, "Not Found" };
  sendJsonError(request, error);
}

void getHome(AsyncWebServerRequest* request) {
  AsyncResponseStream* response = request->beginResponseStream("text/html");
  response->print("<!DOCTYPE HTML><html><body><h1>It works with async!</h1></body></html>");
  request->send(response);
}

void getSettings(AsyncWebServerRequest* request) {
  sendJsonSettings(request);
}

void putSettings(AsyncWebServerRequest* request, JsonVariant& json) {
  const JsonObject& root = json.as<JsonObject>();
  if (root.isNull()) {
    HttpError error { HTTP_BAD_REQUEST, "invalid JSON payload" };
    sendJsonError(request, error);
    return;
  }

  int ledNew = root["led"] | -1;
  if (ledNew == LOW || ledNew == HIGH) {
    led = ledNew;
    digitalWrite(ESP8266_LED, led);
  } else {
    HttpError error { HTTP_BAD_REQUEST, "invalid or missing LED value" };
    sendJsonError(request, error);
    return;
  }

  sendJsonSettings(request);
}

void initServer() {
  server.on("/", HTTP_GET, getHome);

  server.on("/settings", HTTP_GET, getSettings);

  AsyncCallbackJsonWebHandler* putSettingsHandler = new AsyncCallbackJsonWebHandler("/settings", putSettings);
  putSettingsHandler->setMethod(HTTP_PUT);
  server.addHandler(putSettingsHandler);

  server.onNotFound(notFound);

  server.begin();
}

// MAIN FUNCTIONS

void setup() {
  initHardware();
  if (initWifi() != WL_CONNECTED) {
    return;
  }
  initServer();
}

void loop() {}
