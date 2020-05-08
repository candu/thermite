#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>

#include "private.h"

#define PIN_ONE_WIRE 0
#define PIN_LED 5

#define HTTP_OK 200
#define HTTP_BAD_REQUEST 400
#define HTTP_NOT_FOUND 404

#define TEMP_RESOLUTION 11
#define TEMP_REQUEST_DELAY 750 / (1 << (12 - TEMP_RESOLUTION))
#define TEMP_REQUEST_INTERVAL 60000

AsyncWebServer server(80);

OneWire oneWire(PIN_ONE_WIRE);
DallasTemperature thermometerManager(&oneWire);

DeviceAddress thermometer;

uint8_t led = LOW;
float temp = DEVICE_DISCONNECTED_C;
unsigned long lastRequestedAt = 0ul;

struct HttpError {
  uint16_t code;
  const char* message;
};

const char index_html[] PROGMEM =
  "<!DOCTYPE HTML>"
  "<html><head>"
    "<meta charset=\"utf-8\">"
    "<link href=\"http://192.168.0.13:8000/main.css\" rel=\"stylesheet\">"
  "</head><body>"
    "<h1>It works with async from PROGMEM!</h1>"
    "<script src=\"http://192.168.0.13:8000/main.js\"></script>"
  "</body></html>";

// HARDWARE SETUP

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

bool initThermometer() {
  if (!thermometerManager.getAddress(thermometer, 0)) {
    Serial.println("Could not get thermometer address!");
    return false;  
  }
  Serial.print("Thermometer address: ");
  printAddress(thermometer);
  Serial.println();

  if (OneWire::crc8(thermometer, 7) != thermometer[7]) {
    Serial.println("Invalid address CRC!");
    return false;
  }

  thermometerManager.setResolution(thermometer, TEMP_RESOLUTION);
  thermometerManager.setWaitForConversion(false);
  return true;
}

bool initHardware() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  Serial.begin(115200);
  Serial.println();

  return initThermometer();
}

// WIFI CONNECTION SETUP

uint8_t initWifiConnection() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PW);

  Serial.print("Connecting");
  uint8_t i = 0;
  while (true) {
    // check wifi status
    uint8_t status = WiFi.status();
    if (status == WL_NO_SSID_AVAIL
      || status == WL_CONNECTED
      || status == WL_CONNECT_FAILED) {
      return status;
    }

    // blink LED, print to serial
    led = i % 2 == 0 ? HIGH : LOW;
    digitalWrite(PIN_LED, led);
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

  digitalWrite(PIN_LED, LOW);
  Serial.println();
}

uint8_t initWifi() {
  uint8_t status = initWifiConnection();
  notifyWifiStatus(status);
  return status;
}

// WEB SERVER SETUP

void sendJsonError(AsyncWebServerRequest* request, const HttpError& error) {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  response->setCode(error.code);

  const JsonObject& root = response->getRoot();
  root["code"] = error.code;
  root["message"] = error.message;

  response->setLength();
  request->send(response);
}

void sendJsonSettings(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse();

  const JsonObject& root = response->getRoot();
  root["led"] = led;
  root["temp"] = temp;

  response->setLength();
  request->send(response);
}

void notFound(AsyncWebServerRequest* request) {
  HttpError error { HTTP_NOT_FOUND, "Not Found" };
  sendJsonError(request, error);
}

void getHome(AsyncWebServerRequest* request) {
  request->send_P(200, "text/html", index_html);
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

  uint8_t ledNew = root["led"] | -1;
  if (ledNew == LOW || ledNew == HIGH) {
    led = ledNew;
    digitalWrite(PIN_LED, led);
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
  if (!initHardware()) {
    return;
  }
  if (initWifi() != WL_CONNECTED) {
    return;
  }
  initServer();
}

void loop() {
  unsigned long now = millis();
  if (lastRequestedAt == 0ul || (now - lastRequestedAt > TEMP_REQUEST_INTERVAL)) {
    thermometerManager.requestTemperatures();
    lastRequestedAt = now;
  } else if (now - lastRequestedAt > TEMP_REQUEST_DELAY) {
    float tempNew = thermometerManager.getTempC(thermometer);
    if (tempNew == DEVICE_DISCONNECTED_C) {
      Serial.println("Could not read temperature!");
    } else {
      temp = tempNew;
    }
  }
  
  delay(100);
}
