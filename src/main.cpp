#include <Arduino.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <OneWire.h>
#include <Timezone.h>
#include <WiFiUdp.h>

#include "private.h"
#include "ThermiteInternalState.h"
#include "ThermiteUserSettingsManager.h"
#include "ThermiteWebController.h"

#define PIN_ONE_WIRE 0
#define PIN_WIFI_INDICATOR 4
#define PIN_HEATER 5

#define LOOP_INTERVAL 100ul

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
Timezone timezone(EDT, EST);

/**
 * Controls the wifi indicator light as follows:
 * 
 * - intermittent flashing: connecting to wifi;
 * - off: wifi connected;
 * - on: wifi connection error.
 */
uint8_t wifiIndicator = LOW;

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
ThermiteUserSettingsManager userSettingsManager;
ThermiteInternalState internalState(
  userSettingsManager,
  thermometerManager,
  ntpClient,
  timezone
);
ThermiteWebController webController(userSettingsManager, internalState);

// HARDWARE

bool initHardware() {
  pinMode(PIN_WIFI_INDICATOR, OUTPUT);
  pinMode(PIN_HEATER, OUTPUT);

  digitalWrite(PIN_WIFI_INDICATOR, LOW);
  digitalWrite(PIN_HEATER, LOW);

  Serial.begin(115200);
  Serial.println();

  bool result = internalState.init();
  if (!result) {
    Serial.println("Could not initialize thermometer!");
  }
  return result;
}

void updateHardware() {
  bool heater = internalState.getHeater();
  digitalWrite(PIN_HEATER, heater ? HIGH : LOW);
};

// WIFI

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
    Serial.print("Invalid wifi SSID!");
  } else if (status == WL_CONNECTED) {
    Serial.print("Connected to wifi, IP address: ");
    Serial.println(WiFi.localIP());
  } else if (status == WL_CONNECT_FAILED) {
    Serial.println("Invalid wifi password!");
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

// MAIN FUNCTIONS

void setup() {
  if (!initHardware()) {
    return;
  }
  if (initWifi() != WL_CONNECTED) {
    return;
  }
  ntpClient.begin();
  webController.initRoutes(server);

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
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

  internalState.update(startOfLoop);
  updateHardware();

  unsigned long endOfLoop = millis();
  unsigned long loopDelay = getLoopDelay(startOfLoop, endOfLoop);
  delay(loopDelay);
}
