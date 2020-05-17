#ifndef _THERMITE_USER_SETTINGS_MANAGER_H__
#define _THERMITE_USER_SETTINGS_MANAGER_H__

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Jsonable.h"

struct ThermiteSetPoint : public Jsonable {
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
};

struct ThermiteDailySchedule : public Jsonable {
  /**
   * Each daily schedule can be given a name of up to 15 characters in length.
   */
  char _name[16];

  /**
   * Each daily schedule is divided into 96 15-minute intervals.  The user can assign one
   * of the four configured set points to each of these intervals.
   * 
   * These 15-minute intervals are encoded in 2 bits each, for a total of 24 bytes.
   */
  uint8_t _schedule[24];

  ThermiteDailySchedule(const char* name, const uint8_t* schedule);
  bool toJSON(const JsonObject& root) const;
};

struct ThermiteUserSettingsManager : public Jsonable {
  /**
   * `thermite` supports four user-configurable temperature set points.
   */
  ThermiteSetPoint _setPoints[4];

  /**
   * `thermite` supports four user-configurable daily schedules.
   */
  ThermiteDailySchedule _dailySchedules[4];

  /**
   * `thermite` supports two weekly schedules: one main schedule and one temporary schedule.
   * 
   * The weekly schedule consists of 7 days.  The user can assign one of the four configured
   * daily schedules to each of these days.
   * 
   * Daily schedules are encoded in 2 bits each, for a total of 14 bits.
   */
  uint16_t _weeklySchedule;
  uint16_t _weeklyScheduleTemporary;

  /* 
   * The temporary schedule can be set to start and end at specific times (e.g. for the
   * remainder of this week, during all of next week, etc.)
   */
  time_t _temporaryStart;
  time_t _temporaryEnd;

  /**
   * Is `thermite` in vacation mode?
   * 
   * In vacation mode, the weekly schedule is ignored, as is any with the vacation set point target
   * temperature used throughout the day instead.
   */
  bool _vacation;
  int _vacationSetPointIndex;

  ThermiteUserSettingsManager();

  float getTargetTemperature(time_t t) const;
  bool toJSON(const JsonObject& root) const;
};

#endif