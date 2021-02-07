#include <Arduino.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <OneWire.h>
#include <SparkFun_Qwiic_Relay.h>
#include <Timezone.h>
#include <WiFiUdp.h>
#include <Wire.h>

#include "private.h"
#include "ThermiteInternalState.h"
#include "ThermiteUserSettingsManager.h"
#include "ThermiteWebController.h"

#define PIN_ONE_WIRE 0
#define PIN_LED_INDICATOR 4
#define RELAY_ADDR_HEATER 0x18

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
 * Qwiic relay to control the heater.
 */
Qwiic_Relay heaterManager(RELAY_ADDR_HEATER);


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

/**
 * Indicates whether thermite is connected to an actual heater.  We set this flag below
 * in `setup()`, and use it in `loop()` to avoid doing work if initialization didn't succeed.
 */
bool heaterConnected = false;

// HARDWARE

bool initHardware() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println();

  pinMode(PIN_LED_INDICATOR, OUTPUT);
  digitalWrite(PIN_LED_INDICATOR, LOW);

  if (!internalState.init()) {
    Serial.println("Could not initialize thermometer!");
    return false;
  }

  heaterConnected = heaterManager.begin();
  if (heaterConnected) {
    Serial.println("Could not initialize heater relay!");
  }

  return true;
}

void updateHardware() {
  bool heater = internalState.getHeater();
  if (heater) {
    if (heaterConnected) {
      heaterManager.turnRelayOn();
    }
    digitalWrite(PIN_LED_INDICATOR, HIGH);
  } else {
    if (heaterConnected) {
      heaterManager.turnRelayOff();
    }
    digitalWrite(PIN_LED_INDICATOR, LOW);
  }
}

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
    digitalWrite(PIN_LED_INDICATOR, wifiIndicator);
    delay(100);
    if (i % 5 == 0) {
      Serial.print(".");
    }
    i++;
  }
}

void notifyWifiStatus(uint8_t status) {
  if (status == WL_NO_SSID_AVAIL) {
    Serial.println("Invalid wifi SSID!");
  } else if (status == WL_CONNECTED) {
    Serial.print("Connected to wifi, IP address: ");
    Serial.println(WiFi.localIP());
  } else if (status == WL_CONNECT_FAILED) {
    Serial.println("Invalid wifi password!");
  }

  wifiIndicator = status == WL_CONNECTED ? LOW : HIGH;
  digitalWrite(PIN_LED_INDICATOR, wifiIndicator);

  Serial.println();
}

uint8_t initWifi() {
  uint8_t status = initWifiConnection();
  notifyWifiStatus(status);
  return status;
}

bool initAll() {
  delay(2000);

  if (!initHardware()) {
    return false;
  }
  if (initWifi() != WL_CONNECTED) {
    return false;
  }
  ntpClient.begin();
  webController.initRoutes(server);

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "PUT,GET,OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Max-Age", "600");
  server.begin();

  return true;
}

// MAIN FUNCTIONS

void setup() {
  initAll();
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
