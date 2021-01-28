#include <TimeLib.h>

#include "ThermiteUserSettingsManager.h"

#define SET_POINT_MIN 10
#define SET_POINT_MAX 30
#define WEEKLY_SCHEDULE_MAX 0x3fff

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

bool ThermiteSetPoint::validateJSON(const JsonObject& root) const {
  if (root.containsKey("name")) {
    if (!root["name"].is<const char*>()) {
      return false;
    }
    const char* name = root["name"].as<const char*>();
    size_t nameLen = strnlen(name, 16);
    if (nameLen == 0 || nameLen == 16) {
      return false;
    }
  }
  if (root.containsKey("tempTarget")) {
    if (!root["tempTarget"].is<float>()) {
      return false;
    }
    float tempTarget = root["tempTarget"].as<float>();
    if (tempTarget < SET_POINT_MIN || SET_POINT_MAX < tempTarget) {
      return false;
    }
  }
  return true;
}

void ThermiteSetPoint::updateFromJSON(const JsonObject& root) {
  if (root.containsKey("name")) {
    const char* name = root["name"].as<const char*>();
    strncpy(_name, name, 15);
    _name[15] = '\0';
  }
  if (root.containsKey("tempTarget")) {
    float tempTarget = root["tempTarget"].as<float>();
    _tempTarget = tempTarget;
  }
}

ThermiteDailySchedule::ThermiteDailySchedule(const char* name, const uint8_t* schedule) {
  strncpy(_name, name, 15);
  _name[15] = '\0';

  memcpy(_schedule, schedule, 12 * sizeof(uint8_t));
}

bool ThermiteDailySchedule::toJSON(const JsonObject& root) const {
  if (!root["name"].set(_name)) {
    return false;
  }
  const JsonArray& jsonSchedule = root.createNestedArray("schedule");
  if (jsonSchedule.isNull()) {
    return false;
  }
  for (int i = 0; i < 12; i++) {
    if (!jsonSchedule.add(_schedule[i])) {
      return false;
    }
  }
  return true;
}

bool ThermiteDailySchedule::validateJSON(const JsonObject& root) const {
  if (root.containsKey("name")) {
    if (!root["name"].is<const char*>()) {
      return false;
    }
    const char* name = root["name"].as<const char*>();
    size_t nameLen = strnlen(name, 16);
    if (nameLen == 0 || nameLen == 16) {
      return false;
    }
  }
  if (root.containsKey("schedule")) {
    if (!root["schedule"].is<JsonArray>()) {
      return false;
    }
    JsonArray schedule = root["schedule"].as<JsonArray>();
    if (schedule.size() != 12) {
      return false;
    }
    for (const JsonVariant& scheduleByte : schedule) {
      if (!scheduleByte.is<uint8_t>()) {
        return false;
      }
    }
  }
  return true;
}

void ThermiteDailySchedule::updateFromJSON(const JsonObject& root) {
  if (root.containsKey("name")) {
    const char* name = root["name"].as<const char*>();
    strncpy(_name, name, 15);
    _name[15] = '\0';
  }
  if (root.containsKey("schedule")) {
    JsonArray schedule = root["schedule"].as<JsonArray>();
    int i = 0;
    for (const JsonVariant& scheduleByte : schedule) {
      uint8_t scheduleByteValue = scheduleByte.as<uint8_t>();
      _schedule[i] = scheduleByteValue;
      i++;
    }
  }
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
        0xaa, 0xaa, 0xaa,
        0x5a, 0x00, 0x00,
        0x00, 0x00, 0x50,
        0x55, 0xa5, 0xaa
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
        0xaa, 0xaa, 0xaa,
        0x5a, 0x55, 0x55,
        0x55, 0x55, 0x55,
        0x55, 0xa5, 0xaa
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
        0xaa, 0xaa, 0xaa,
        0xaa, 0x55, 0x55,
        0x55, 0x55, 0x55,
        0x55, 0x55, 0xaa
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
        0xaa, 0xaa, 0xaa,
        0xaa, 0x55, 0x55,
        0x55, 0x55, 0x55,
        0x55, 0x55, 0xaa
      }
    }
  }),
  _weeklySchedule(0x2002),
  _tempOverride(17.0f),
  _overrideStart(0l),
  _overrideEnd(0l) {}

float ThermiteUserSettingsManager::getTargetTemperature(time_t t) const {
  // resolve temperature override
  if (_overrideStart <= t && t < _overrideEnd) {
    return _tempOverride;
  }

  // resolve weekly schedule
  int d = weekday(t) - 1;
  int i = (_weeklySchedule >> (d * 2)) & 0x3;

  // resolve daily schedule
  int h = hour(t);
  int m = minute(t);
  m = (m / 30) << 1;
  if (h & 0x1) {
    m += 4;
  }
  h >>= 1;
  i = (_dailySchedules[i]._schedule[h] >> m) & 0x3;
  
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
  if (!root["tempOverride"].set(_tempOverride)) {
    return false;
  }
  if (!root["overrideStart"].set(_overrideStart)) {
    return false;
  }
  if (!root["overrideEnd"].set(_overrideEnd)) {
    return false;
  }
  return true;
}

bool ThermiteUserSettingsManager::validateJSON(const JsonObject& root) const {
  if (root.containsKey("setPoints")) {
    if (!root["setPoints"].is<JsonArray>()) {
      return false;
    }
    JsonArray setPoints = root["setPoints"].as<JsonArray>();
    if (setPoints.size() != 4) {
      return false;
    }
    int i = 0;
    for (const JsonVariant& setPoint : setPoints) {
      if (!setPoint.is<JsonObject>()) {
        return false;
      }
      const JsonObject& setPointRoot = setPoint.as<JsonObject>();
      if (!_setPoints[i].validateJSON(setPointRoot)) {
        return false;
      }
      i++;
    }
  }
  if (root.containsKey("dailySchedules")) {
    if (!root["dailySchedules"].is<JsonArray>()) {
      return false;
    }
    JsonArray dailySchedules = root["dailySchedules"].as<JsonArray>();
    if (dailySchedules.size() != 4) {
      return false;
    }
    int i = 0;
    for (const JsonVariant& dailySchedule : dailySchedules) {
      if (!dailySchedule.is<JsonObject>()) {
        return false;
      }
      const JsonObject& dailyScheduleRoot = dailySchedule.as<JsonObject>();
      if (!_dailySchedules[i].validateJSON(dailyScheduleRoot)) {
        return false;
      }
      i++;
    }
  }
  if (root.containsKey("weeklySchedule")) {
    if (!root["weeklySchedule"].is<uint16_t>()) {
      return false;
    }
    uint16_t weeklySchedule = root["weeklySchedule"].as<uint16_t>();
    if (weeklySchedule > WEEKLY_SCHEDULE_MAX) {
      return false;
    }
  }
  if (root.containsKey("tempOverride")) {
    if (!root["tempOverride"].is<float>()) {
      return false;
    }
    float tempOverride = root["tempOverride"].as<float>();
    if (tempOverride < SET_POINT_MIN || SET_POINT_MAX < tempOverride) {
      return false;
    }
  }
  if (root.containsKey("overrideStart") || root.containsKey("overrideEnd")) {
    if (!root["overrideStart"].is<time_t>() || !root["overrideEnd"].is<time_t>()) {
      return false;
    }
    time_t overrideStart = root["overrideStart"].as<time_t>();
    time_t overrideEnd = root["overrideEnd"].as<time_t>();
    if (overrideStart == 0l && overrideEnd != 0l) {
      return false;
    }
    if (overrideStart != 0l && overrideEnd == 0l) {
      return false;
    }
    if (overrideStart > overrideEnd) {
      return false;
    }
  }
  return true;
}

void ThermiteUserSettingsManager::updateFromJSON(const JsonObject& root) {
  if (root.containsKey("setPoints")) {
    JsonArray setPoints = root["setPoints"].as<JsonArray>();
    int i = 0;
    for (const JsonVariant& setPoint : setPoints) {
      const JsonObject& setPointRoot = setPoint.as<JsonObject>();
      _setPoints[i].updateFromJSON(setPointRoot);
      i++;
    }
  }
  if (root.containsKey("dailySchedules")) {
    JsonArray dailySchedules = root["dailySchedules"].as<JsonArray>();
    int i = 0;
    for (const JsonVariant& dailySchedule : dailySchedules) {
      const JsonObject& dailyScheduleRoot = dailySchedule.as<JsonObject>();
      _dailySchedules[i].updateFromJSON(dailyScheduleRoot);
      i++;
    }
  }
  if (root.containsKey("weeklySchedule")) {
    uint16_t weeklySchedule = root["weeklySchedule"].as<uint16_t>();
    _weeklySchedule = weeklySchedule;
  }
  if (root.containsKey("tempOverride")) {
    float tempOverride = root["tempOverride"].as<float>();
    _tempOverride = tempOverride;
  }
  if (root.containsKey("overrideStart") || root.containsKey("overrideEnd")) {
    time_t overrideStart = root["overrideStart"].as<time_t>();
    _overrideStart = overrideStart;

    time_t overrideEnd = root["overrideEnd"].as<time_t>();
    _overrideEnd = overrideEnd;
  }
}