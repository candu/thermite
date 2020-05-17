#ifndef _THERMITE_INTERNAL_STATE_H__
#define _THERMITE_INTERNAL_STATE_H__

#include <Arduino.h>
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <Timezone.h>

#include "Jsonable.h"
#include "ThermiteUserSettingsManager.h"

#define DATE_TIME_ISO_LEN 26
#define TEMP_RESOLUTION 11
#define TEMP_REQUEST_DELAY 750ul / (1ul << (12 - TEMP_RESOLUTION))
#define TEMP_REQUEST_INTERVAL 60000ul

class ThermiteInternalState : public Jsonable {
private:
  const ThermiteUserSettingsManager& _userSettingsManager;
  DallasTemperature& _thermometerManager;
  NTPClient& _ntpClient;
  Timezone& _timezone;

  DeviceAddress _thermometer;

  /**
   * Used to store date and time in the ISO 8601-compliant format `"2020-05-09T10:58:27-04:00"`.
   */
  char _dateTimeIso[DATE_TIME_ISO_LEN];
  
  /**
   * Should the heater be on?
   */
  bool _heater;

  /**
   * Last measured temperature, in degrees Celsius.
   * 
   * As per the DS18B20 datasheet, we must wait `TEMP_REQUEST_DELAY` ms after the last
   * temperature request to read thhe updated measurement.
   */
  float _temp;

  /**
   * Timestamp at which the last temperature reading was measured.
   * 
   * To keep these regularly scheduled regardless of NTP reachability, clock drift, etc. we use
   * `millis()` to record this from the internal clock.
   */
  unsigned long _tempLastRequestedAt;

  /**
   * Last measured target temperature, in degrees Celsius.
   * 
   * This is used to control `_heater`, as described below.
   */
  float _tempTarget;

  /**
   * Hysteresis threshold for temperature targets.
   * 
   * When the heater is off and the temperature is below `_tempTarget - _tempHysteresis`, the heater
   * is turned on.  When the heater is on and the temperature is above `_tempTarget + _tempHysteresis`,
   * the heater is turned off.
   */
  float _tempHysteresis;

  void _updateHeater();
  void _updateTargetTemperature();
  void _updateTemperature(unsigned long updateAt);
public:
  ThermiteInternalState(
    const ThermiteUserSettingsManager& userSettingsManager,
    DallasTemperature& thermometerManager,
    NTPClient& ntpClient,
    Timezone& timezone
  );

  bool getHeater() const { return _heater; }
  bool init();
  bool toJSON(const JsonObject& root) const;
  void update(unsigned long updateAt);
  void updateDateTimeIso();
};

#endif