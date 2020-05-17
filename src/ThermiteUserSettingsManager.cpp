#include <AsyncJson.h>
#include <TimeLib.h>

#include "ThermiteUserSettingsManager.h"

ThermiteSetPoint::ThermiteSetPoint(const char name[16], float tempTarget)
: _tempTarget(tempTarget) {
  strncpy(_name, name, 15);
  _name[15] = '\0';
}

bool ThermiteSetPoint::toJSON(const JsonObject& root) const {
  if (!root["name"].set(_name)) {
    return false;
  }
  if (!root["tempTarget"].set(_tempTarget)) {
    return false;
  }
  return true;
}

ThermiteDailySchedule::ThermiteDailySchedule(const char* name, const uint8_t* schedule) {
  strncpy(_name, name, 15);
  _name[15] = '\0';

  memcpy(_schedule, schedule, 24 * sizeof(uint8_t));
}

bool ThermiteDailySchedule::toJSON(const JsonObject& root) const {
  if (!root["name"].set(_name)) {
    return false;
  }
  const JsonArray& jsonSchedule = root.createNestedArray("schedule");
  if (jsonSchedule.isNull()) {
    return false;
  }
  for (int i = 0; i < 24; i++) {
    if (!jsonSchedule.add(_schedule[i])) {
      return false;
    }
  }
  return true;
}

ThermiteUserSettingsManager::ThermiteUserSettingsManager()
: _setPoints({
    { "Home Office", 20.0f },
    { "Normal", 17.0f },
    { "Sleep", 16.0f },
    { "Vacation", 14.0f },
  }),
  _dailySchedules({
    {
      "Work from Home",
      (const uint8_t[]) {
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
      (const uint8_t[]) {
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
      (const uint8_t[]) {
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
      (const uint8_t[]) {
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
    }
  }),
  _weeklySchedule(0x2002),
  _weeklyScheduleTemporary(0x2002),
  _temporaryStart(0l),
  _temporaryEnd(0l),
  _vacation(false),
  _vacationSetPointIndex(3) {}

float ThermiteUserSettingsManager::getTargetTemperature(time_t t) const {
  if (_vacation) {
    return _setPoints[_vacationSetPointIndex]._tempTarget;
  }

  // resolve weekly schedule
  int d = weekday(t) - 1;
  int i;
  if (_temporaryStart <= t && t < _temporaryEnd) {
    i = (_weeklyScheduleTemporary >> (d * 2)) & 0x3;
  } else {
    i = (_weeklySchedule >> (d * 2)) & 0x3;
  }

  // resolve daily schedule
  int h = hour(t);
  int m = minute(t);
  i = (_dailySchedules[i]._schedule[h] >> ((m / 15) * 2)) & 0x3;
  
  return _setPoints[i]._tempTarget;
}

bool ThermiteUserSettingsManager::toJSON(const JsonObject& root) const {
  const JsonArray& jsonSetPoints = root.createNestedArray("setPoints");
  if (jsonSetPoints.isNull()) {
    return false;
  }
  for (int i = 0; i < 4; i++) {
    const JsonObject& jsonSetPoint = jsonSetPoints.createNestedObject();
    if (jsonSetPoint.isNull()) {
      return false;
    }
    if (!_setPoints[i].toJSON(jsonSetPoint)) {
      return false;
    }
  }

  const JsonArray& jsonDailySchedules = root.createNestedArray("dailySchedules");
  if (jsonDailySchedules.isNull()) {
    return false;
  }
  for (int i = 0; i < 4; i++) {
    const JsonObject& jsonDailySchedule = jsonDailySchedules.createNestedObject();
    if (jsonDailySchedule.isNull()) {
      return false;
    }
    if (!_dailySchedules[i].toJSON(jsonDailySchedule)) {
      return false;
    }
  }

  if (!root["weeklySchedule"].set(_weeklySchedule)) {
    return false;
  }
  if (!root["weeklyScheduleTemporary"].set(_weeklyScheduleTemporary)) {
    return false;
  }
  if (!root["temporaryStart"].set(_temporaryStart)) {
    return false;
  }
  if (!root["temporaryEnd"].set(_temporaryEnd)) {
    return false;
  }
  if (!root["vacation"].set(_vacation)) {
    return false;
  }
  if (!root["vacationSetPointIndex"].set(_vacationSetPointIndex)) {
    return false;
  }
  return true;
}