#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <OneWire.h>
#include <Timezone.h>
#include <WiFiUdp.h>

#include "private.h"

#define PIN_ONE_WIRE 0
#define PIN_WIFI_INDICATOR 4
#define PIN_HEATER 5

#define HTTP_OK 200
#define HTTP_BAD_REQUEST 400
#define HTTP_NOT_FOUND 404

#define LOOP_INTERVAL 100ul
#define TEMP_RESOLUTION 11
#define TEMP_REQUEST_DELAY 750ul / (1ul << (12 - TEMP_RESOLUTION))
#define TEMP_REQUEST_INTERVAL 60000ul

/**
 * Web server, used both to load the thermostat web UI and as a REST API to change settings.
 * 
 * The web UI itself is a minimal on-board page (see `index_html` below) that functions as an
 * entry point.  It references `thermite` CSS / JS resources stored on a third-party server;
 * these resources then render the actual interface.
 * 
 * The interface itself is designed to use the on-board REST API; no user information is ever
 * sent to the third-party static resources server.
 */
AsyncWebServer server(80);

/**
 * OneWire bus for the DS18B20 thermometer.  We only have a single thermometer on the bus,
 * but we could attach more - for instance, some heating / cooling systems take outdoor
 * temperature into account to help save energy.
 * 
 * `thermometerManager` wraps the OneWire bus `oneWire` to provide functionality specific
 * to DS18B20 thermometers - many other types of devices use the OneWire protocol.
 * 
 * `thermometer` is the device address of our single thermometer, and is used with
 * `thermometerManager` to read the current temperature.
 */
OneWire oneWire(PIN_ONE_WIRE);
DallasTemperature thermometerManager(&oneWire);
DeviceAddress thermometer;

/**
 * NTP client, used so that we can schedule temperature changes according to real-world
 * time.
 * 
 * Since the ESP8266 provides us with a wifi connection, we can use NTP instead of installing
 * a separate real-time clock (RTC) module.
 */
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "pool.ntp.org");

/**
 * Hardcoded timezone information, currently set to Eastern Time with DST transitions.
 * Change this if you're installing `thermite` in a different time zone.
 */
const TimeChangeRule EDT = {"EDT", Second, Sun, Mar, 2, -240};
const TimeChangeRule EST = {"EST", First, Sun, Nov, 2, -300};
const Timezone TIMEZONE(EDT, EST);

/**
 * Controls the wifi indicator light as follows:
 * 
 * - intermittent flashing: connecting to wifi;
 * - off: wifi connected;
 * - on: wifi connection error.
 */
uint8_t wifiIndicator = LOW;

/**
 * Controls the heater via the Quiic relay.
 */
uint8_t heater = LOW;

/**
 * What was the temperature last time it was measured?
 * 
 * This is measured in degrees Celsius.  As per the DS18B20 datasheet, we 
 */
float temp = DEVICE_DISCONNECTED_C;

/**
 * When was the temperature last requested?
 * 
 * To keep these regularly scheduled regardless of NTP reachability, clock drift, etc. we use
 * `millis()` to record this from the internal clock.
 */
unsigned long lastRequestedAt = 0ul;

/**
 *
 */
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
  pinMode(PIN_WIFI_INDICATOR, OUTPUT);
  pinMode(PIN_HEATER, OUTPUT);

  digitalWrite(PIN_WIFI_INDICATOR, LOW);
  digitalWrite(PIN_HEATER, LOW);

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
    wifiIndicator = i % 2 == 0 ? HIGH : LOW;
    digitalWrite(PIN_WIFI_INDICATOR, wifiIndicator);
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

  wifiIndicator = status == WL_CONNECTED ? LOW : HIGH;
  digitalWrite(PIN_WIFI_INDICATOR, wifiIndicator);

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
  root["heater"] = heater;
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

  // TODO: the heater should *not* be manually controllable here!
  uint8_t heaterNew = root["heater"] | -1;
  if (heaterNew == LOW || heaterNew == HIGH) {
    heater = heaterNew;
    digitalWrite(PIN_HEATER, heater);
  } else {
    HttpError error { HTTP_BAD_REQUEST, "invalid or missing heater value" };
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

void updateTemperature(unsigned long startOfLoop) {
  if (startOfLoop < lastRequestedAt) {
    /*
     * Underflow condition: `millis()` wraps around to zero every 50 days or so.  To
     * make sure the board can continue to function, we manually set `lastRequestedAt`
     * to `now`; this will cause the next request interval to happen later than usual.
     */
    lastRequestedAt = startOfLoop;
  } else if (lastRequestedAt == 0ul || (startOfLoop - lastRequestedAt > TEMP_REQUEST_INTERVAL)) {
    /*
     * We've either never requested a temperature (`lastRequestedAt == 0ul`), or the
     * request interval has elapsed.  Either way, request a new temperature reading.
     */
    thermometerManager.requestTemperatures();
    lastRequestedAt = startOfLoop;
  } else if (startOfLoop - lastRequestedAt > TEMP_REQUEST_DELAY) {
    /*
     * To read the DS18B20, you have to wait at least `TEMP_REQUEST_DELAY` ms after
     * the previous temperature reading request.
     */
    float tempNew = thermometerManager.getTempC(thermometer);
    if (tempNew == DEVICE_DISCONNECTED_C) {
      Serial.println("Could not read temperature!");
    } else {
      temp = tempNew;
    }
  }
}

unsigned long getLoopDelay(unsigned long startOfLoop, unsigned long endOfLoop) {
  if (endOfLoop < startOfLoop) {
    // Underflow condition: wait for whole interval
    return LOOP_INTERVAL;
  }
  unsigned long elapsed = endOfLoop - startOfLoop;
  if (elapsed > LOOP_INTERVAL) {
    return 0;
  }
  return LOOP_INTERVAL - elapsed;
}

void loop() {
  unsigned long startOfLoop = millis();

  updateTemperature(startOfLoop);

  unsigned long endOfLoop = millis();
  unsigned long loopDelay = getLoopDelay(startOfLoop, endOfLoop);
  delay(loopDelay);
}
