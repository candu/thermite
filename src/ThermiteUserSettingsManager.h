#ifndef _THERMITE_USER_SETTINGS_MANAGER_H__
#define _THERMITE_USER_SETTINGS_MANAGER_H__

#include <Arduino.h>
#include <ArduinoJson.h>

#include "JsonIO.h"

struct ThermiteSetPoint : public JsonRead, public JsonWrite {
  /**
   * Each set point can be given a name of up to 15 characters in length.
   */
  char _name[16];

  /**
   * Each set point has a target temperature.
   */
  float _tempTarget;

  ThermiteSetPoint(const char* name, float tempTarget);

  bool toJSON(const JsonObject& root) const;
  bool validateJSON(const JsonObject& root) const;
  void updateFromJSON(const JsonObject& root);
};

struct ThermiteDailySchedule : public JsonRead, public JsonWrite {
  /**
   * Each daily schedule can be given a name of up to 15 characters in length.
   */
  char _name[16];

  /**
   * Each daily schedule is divided into 48 30-minute intervals.  The user can assign one
   * of the four configured set points to each of these intervals.
   * 
   * These 30-minute intervals are encoded in 2 bits each, for a total of 12 bytes.
   */
  uint8_t _schedule[12];

  ThermiteDailySchedule(const char* name, const uint8_t* schedule);
  bool toJSON(const JsonObject& root) const;
  bool validateJSON(const JsonObject& root) const;
  void updateFromJSON(const JsonObject& root);
};

struct ThermiteUserSettingsManager : public JsonRead, public JsonWrite {
  /**
   * `thermite` supports four user-configurable temperature set points.
   */
  ThermiteSetPoint _setPoints[4];

  /**
   * `thermite` supports four user-configurable daily schedules.
   */
  ThermiteDailySchedule _dailySchedules[4];

  /**
   * `thermite` supports one weekly schedule.
   * 
   * The weekly schedule consists of 7 days.  The user can assign one of the four configured
   * daily schedules to each of these days.
   * 
   * Daily schedules are encoded in 2 bits each, for a total of 14 bits.
   */
  uint16_t _weeklySchedule;

  /**
   * `thermite` supports a temporary temperature override, which can be used both for
   * short-term overrides (e.g. "set to 20 Celsius for the next hour") and longer-term
   * "vacation mode" overrides (e.g. "I'm gone for two weeks, keep it at this
   * temperature.")
   */
  float _tempOverride;

  /* 
   * Temperature overrides are temporary: they start and end at a given time.  Both values
   * must be set to non-zero values for the override to take effect.
   */
  time_t _overrideStart;
  time_t _overrideEnd;

  ThermiteUserSettingsManager();

  float getTargetTemperature(time_t t) const;
  bool toJSON(const JsonObject& root) const;
  bool validateJSON(const JsonObject& root) const;
  void updateFromJSON(const JsonObject& root);
};

#endif