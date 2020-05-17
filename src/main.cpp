#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <OneWire.h>
#include <TimeLib.h>
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

#define DATE_TIME_ISO_LEN 26

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
Timezone TIMEZONE(EDT, EST);

/**
 * Used to store date and time in the ISO 8601-compliant format `"2020-05-09T10:58:27-04:00"`.
 */
char dateTimeIso[DATE_TIME_ISO_LEN];

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
 * Last measured temperature, in degrees Celsius.
 * 
 * As per the DS18B20 datasheet, we must wait `TEMP_REQUEST_DELAY` ms after the last
 * temperature request to read thhe updated measurement.
 */
float temp = DEVICE_DISCONNECTED_C;

/**
 * Target temperature, according to when this was last checked.
 */
float tempTarget = DEVICE_DISCONNECTED_C;

/**
 * Timestamp at which the last temperature reading was measured.
 * 
 * To keep these regularly scheduled regardless of NTP reachability, clock drift, etc. we use
 * `millis()` to record this from the internal clock.
 */
unsigned long lastRequestedAt = 0ul;

/**
 * Hysteresis threshold for temperature targets.
 * 
 * When the heater is off and the temperature is below `tempTarget - tempHysteresis`, the heater
 * is turned on.  When the heater is on and the temperature is above `tempTarget + tempHysteresis`,
 * the heater is turned off.
 */
float tempHysteresis = 1.0f;

/**
 * `thermite` supports four user-configurable temperature set points.
 */
struct ThermiteSetPoint {
  /**
   * Each set point can be given a name of up to 15 characters in length.
   */
  char name[16];

  /**
   * Each set point has a target temperature.
   */
  float tempTarget;
};
ThermiteSetPoint setPoints[4] = {
  { "Home Office", 20.0f },
  { "Normal", 17.0f },
  { "Sleep", 16.0f },
  { "Vacation", 14.0f },
};

/**
 * `thermite` supports four user-configurable daily schedules.
 */
struct ThermiteDailySchedule {
  /**
   * Each daily schedule can be given a name of up to 15 characters in length.
   */
  char name[16];

  /**
   * Each daily schedule is divided into 96 15-minute intervals.  The user can assign one
   * of the four configured set points to each of these intervals.
   * 
   * These 15-minute intervals are encoded in 2 bits each, for a total of 24 bytes.
   */
  uint8_t schedule[24];

  float getTargetTemperature(time_t t) {
    int h = hour(t);
    int m = minute(t);
    int i = (this->schedule[h] >> ((m / 15) * 2)) & 0x3;
    return setPoints[i].tempTarget;
  }
};
ThermiteDailySchedule dailySchedules[4] = {
  {
    "Work from Home",
    {
      /*
       * 0000-0700: 2 (sleep)
       * 0700-0800: 1 (normal)
       * 0800-1700: 0 (home office)
       * 1700-2100: 1 (normal)
       * 2100-0000: 2 (sleep)
       */
      0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
      0xaa, 0x55, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x55,
      0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa
    }
  },
  {
    "At the Office",
    {
      /*
       * 0000-0700: 2 (sleep)
       * 0700-2100: 1 (normal)
       * 2100-0000: 2 (sleep)
       */
      0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
      0xaa, 0x55, 0x55, 0x55, 0x55, 0x55,
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
      0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa
    }
  },
  {
    "Day Off",
    {
      /*
       * 0000-0800: 2 (sleep)
       * 0800-2200: 1 (normal)
       * 2200-0000: 2 (sleep)
       */
      0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
      0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
      0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa
    }
  },
  {
    "Other",
    {
      /*
       * 0000-0800: 2 (sleep)
       * 0800-2200: 1 (normal)
       * 2200-0000: 2 (sleep)
       */
      0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
      0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
      0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa
    }
  },
};

/**
 * `thermite` supports two weekly schedules: one main schedule and one temporary schedule.
 */
struct ThermiteWeeklySchedule {
  /**
   * The weekly schedule consists of 7 days.  The user can assign one of the four configured
   * daily schedules to each of these days.
   * 
   * Daily schedules are encoded in 2 bits each, for a total of 14 bits.
   */
  uint16_t schedule;

  float getTargetTemperature(time_t t) {
    int d = weekday(t) - 1;
    int i = (this->schedule >> (d * 2)) & 0x3;
    return dailySchedules[i].getTargetTemperature(t);
  }
};
ThermiteWeeklySchedule weeklySchedules[2] = {
  {
    /*
     * Sun: (2) day off
     * Mon-Fri: (0) work from home
     * Sat: (2) day off
     */
    0x2002
  },
  {
    /*
     * Sun: (2) day off
     * Mon-Fri: (0) work from home
     * Sat: (2) day off
     */
    0x2002
  },
};

/* 
 * The temporary schedule can be set to start and end at specific times (e.g. for the
 * remainder of this week, during all of next week, etc.)
 */
struct ThermiteScheduleManager {
  bool vacation;
  int vacationSetPointIndex;
  time_t tempStart;
  time_t tempEnd;

  float getTargetTemperature(time_t t) {
    if (this->vacation) {
      return setPoints[this->vacationSetPointIndex].tempTarget;
    }
    if (this->tempStart <= t && t < this->tempEnd) {
      return weeklySchedules[1].getTargetTemperature(t);
    }
    return weeklySchedules[0].getTargetTemperature(t);
  }
};
ThermiteScheduleManager scheduleManager = { false, 3, 0l, 0l };

/**
 * Is `thermite` in vacation mode?
 * 
 * In vacation mode, the weekly schedule is ignored, as is any with the vacation set point target
 * temperature used throughout the day instead.
 */

/**
 * HTTP error message, containing a status code and a human-readable message.
 * 
 * We use this to help send HTTP error responses with JSON payloads, which helps
 * us handle error conditions in the frontend.
 */
struct HttpError {
  uint16_t code;
  const char* message;
};

/**
 * HTML entry point to web UI, returned by `GET /` handler below.
 * 
 * It links to externally hosted `thermite` CSS / JS resources, which inject the `thermite` web
 * application into the `#app` div.
 */
const char index_html[] PROGMEM =
  "<!DOCTYPE HTML>"
  "<html><head>"
    "<meta charset=\"utf-8\">"
    "<link href=\"http://192.168.0.13:8000/main.css\" rel=\"stylesheet\">"
  "</head><body>"
    "<div id=\"app\">"
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

void updateDateTimeIso() {
  time_t tUtc = ntpClient.getEpochTime();
  TimeChangeRule *tcr;
  time_t tLocal = TIMEZONE.toLocal(tUtc, &tcr);

  snprintf(
    dateTimeIso,
    DATE_TIME_ISO_LEN,
    "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
    year(tLocal),
    month(tLocal),
    day(tLocal),
    hour(tLocal),
    minute(tLocal),
    second(tLocal),
    tcr->offset >= 0 ? '+' : '-',
    abs(tcr->offset) / 60,
    abs(tcr->offset) % 60
  );
}

void sendJsonSettings(AsyncWebServerRequest* request) {
  updateDateTimeIso();

  AsyncJsonResponse* response = new AsyncJsonResponse();

  const JsonObject& root = response->getRoot();
  root["dateTime"] = dateTimeIso;
  root["heater"] = heater;
  root["temp"] = temp;
  root["tempTarget"] = tempTarget;

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

  // TODO: update settings???

  sendJsonSettings(request);
}

void initNtp() {
  ntpClient.begin();
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
  initNtp();
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

void updateTargetTemperature() {
  time_t tUtc = ntpClient.getEpochTime();
  time_t tLocal = TIMEZONE.toLocal(tUtc);
  tempTarget = scheduleManager.getTargetTemperature(tLocal);
}

void updateHeater() {
  if (tempTarget == DEVICE_DISCONNECTED_C) {
    /*
     * Wait until we have a valid target temperature; this should be called after
     * `updateTargetTemperature()`.
     */
    return;
  }
  if (heater == HIGH && temp <= tempTarget - tempHysteresis) {
    heater = LOW;
    digitalWrite(PIN_HEATER, heater);
  } else if (heater == LOW && temp >= tempTarget + tempHysteresis) {
    heater = HIGH;
    digitalWrite(PIN_HEATER, heater);
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

  ntpClient.update();
  updateTemperature(startOfLoop);
  updateTargetTemperature();
  updateHeater();

  unsigned long endOfLoop = millis();
  unsigned long loopDelay = getLoopDelay(startOfLoop, endOfLoop);
  delay(loopDelay);
}
