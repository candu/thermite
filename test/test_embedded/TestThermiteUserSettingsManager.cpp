#ifdef UNIT_TEST

#include <Arduino.h>
#include <unity.h>

#include "Constants.h"
#include "ThermiteUserSettingsManager.cpp"

void testSetPointEmpty() {
  ThermiteSetPoint setPoint("foo", 16.0f);

  StaticJsonDocument<CAPACITY_SET_POINT> doc;
  JsonObject root = doc.to<JsonObject>();
  TEST_ASSERT_TRUE(setPoint.validateJSON(root));
}

void testSetPointNameValid() {
  ThermiteSetPoint setPoint("foo", 16.0f);

  StaticJsonDocument<CAPACITY_SET_POINT> doc;
  JsonObject root = doc.to<JsonObject>();
  root["name"] = "foo2";
  TEST_ASSERT_TRUE(setPoint.validateJSON(root));

  setPoint.updateFromJSON(root);
  TEST_ASSERT_EQUAL_STRING(setPoint._name, "foo2");
}

void testSetPointNameEmpty() {
  ThermiteSetPoint setPoint("foo", 16.0f);

  StaticJsonDocument<CAPACITY_SET_POINT> doc;
  JsonObject root = doc.to<JsonObject>();
  root["name"] = "";
  TEST_ASSERT_FALSE(setPoint.validateJSON(root));
}

void testSetPointNameTooLong() {
  ThermiteSetPoint setPoint("foo", 16.0f);

  StaticJsonDocument<CAPACITY_SET_POINT> doc;
  JsonObject root = doc.to<JsonObject>();
  root["name"] = "this name is far, far, far too long";
  TEST_ASSERT_FALSE(setPoint.validateJSON(root));

  setPoint.updateFromJSON(root);
  TEST_ASSERT_EQUAL_STRING(setPoint._name, "this name is fa");
}

void testSetPointTempTargetValid() {
  ThermiteSetPoint setPoint("foo", 16.0f);

  StaticJsonDocument<CAPACITY_SET_POINT> doc;
  JsonObject root = doc.to<JsonObject>();
  root["tempTarget"] = 15.0f;
  TEST_ASSERT_TRUE(setPoint.validateJSON(root));
  setPoint.updateFromJSON(root);
  TEST_ASSERT_EQUAL(setPoint._tempTarget, 15.0f);
}

void testSetPointTempTargetTooLow() {
  ThermiteSetPoint setPoint("foo", 16.0f);

  StaticJsonDocument<CAPACITY_SET_POINT> doc;
  JsonObject root = doc.to<JsonObject>();
  root["tempTarget"] = -100.0f;
  TEST_ASSERT_FALSE(setPoint.validateJSON(root));
}

void testSetPointTempTargetTooHigh() {
  ThermiteSetPoint setPoint("foo", 16.0f);

  StaticJsonDocument<CAPACITY_SET_POINT> doc;
  JsonObject root = doc.to<JsonObject>();
  root["tempTarget"] = 100.0f;
  TEST_ASSERT_FALSE(setPoint.validateJSON(root));
}

void testSetPointBothValid() {
  ThermiteSetPoint setPoint("foo", 16.0f);

  StaticJsonDocument<CAPACITY_SET_POINT> doc;
  JsonObject root = doc.to<JsonObject>();
  root["name"] = "foo3";
  root["tempTarget"] = 17.5f;
  TEST_ASSERT_TRUE(setPoint.validateJSON(root));
  setPoint.updateFromJSON(root);
  TEST_ASSERT_EQUAL_STRING(setPoint._name, "foo3");
  TEST_ASSERT_EQUAL(setPoint._tempTarget, 17.5f);
}

void testSetPointToJson() {
  ThermiteSetPoint setPoint("foo", 16.0f);

  StaticJsonDocument<CAPACITY_SET_POINT> doc;
  JsonObject root = doc.to<JsonObject>();
  TEST_ASSERT_TRUE(setPoint.toJSON(root));
  TEST_ASSERT_EQUAL_STRING(setPoint._name, root["name"]);
  TEST_ASSERT_EQUAL(setPoint._tempTarget, root["tempTarget"]);
  TEST_ASSERT_TRUE(setPoint.validateJSON(root));
}

void testDailyScheduleEmpty() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  TEST_ASSERT_TRUE(dailySchedule.validateJSON(root));
}

void testDailyScheduleNameValid() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  root["name"] = "foo2";
  TEST_ASSERT_TRUE(dailySchedule.validateJSON(root));

  dailySchedule.updateFromJSON(root);
  TEST_ASSERT_EQUAL_STRING(dailySchedule._name, "foo2");
}

void testDailyScheduleNameEmpty() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  root["name"] = "";
  TEST_ASSERT_FALSE(dailySchedule.validateJSON(root));
}

void testDailyScheduleNameTooLong() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  root["name"] = "this name is far, far, far too long";
  TEST_ASSERT_FALSE(dailySchedule.validateJSON(root));

  dailySchedule.updateFromJSON(root);
  TEST_ASSERT_EQUAL_STRING(dailySchedule._name, "this name is fa");
}

void testDailyScheduleScheduleValid() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonSchedule = root.createNestedArray("schedule");
  for (uint8_t i = 0; i < 12; i++) {
    jsonSchedule.add(i);
  }
  TEST_ASSERT_TRUE(dailySchedule.validateJSON(root));

  dailySchedule.updateFromJSON(root);
  for (uint8_t i = 0; i < 12; i++) {
    TEST_ASSERT_EQUAL(dailySchedule._schedule[i], i);
  }
}

void testDailyScheduleScheduleEmpty() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonSchedule = root.createNestedArray("schedule");
  TEST_ASSERT_FALSE(dailySchedule.validateJSON(root));
}

void testDailyScheduleScheduleTooShort() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonSchedule = root.createNestedArray("schedule");
  for (uint8_t i = 0; i < 11; i++) {
    jsonSchedule.add(i);
  }
  TEST_ASSERT_FALSE(dailySchedule.validateJSON(root));
}

void testDailyScheduleScheduleTooLong() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonSchedule = root.createNestedArray("schedule");
  for (uint8_t i = 0; i < 13; i++) {
    jsonSchedule.add(i);
  }
  TEST_ASSERT_FALSE(dailySchedule.validateJSON(root));
}

void testDailyScheduleScheduleNotArray() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  root["schedule"] = "not a schedule";
  TEST_ASSERT_FALSE(dailySchedule.validateJSON(root));
}

void testDailyScheduleScheduleValueTooLow() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonSchedule = root.createNestedArray("schedule");
  for (uint8_t i = 0; i < 11; i++) {
    jsonSchedule.add(i);
  }
  jsonSchedule.add(-1);
  TEST_ASSERT_FALSE(dailySchedule.validateJSON(root));
}

void testDailyScheduleScheduleValueTooHigh() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonSchedule = root.createNestedArray("schedule");
  for (uint8_t i = 0; i < 11; i++) {
    jsonSchedule.add(i);
  }
  jsonSchedule.add(0x100);
  TEST_ASSERT_FALSE(dailySchedule.validateJSON(root));
}

void testDailyScheduleBothValid() {
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  root["name"] = "foo3";
  JsonArray jsonSchedule = root.createNestedArray("schedule");
  for (uint8_t i = 0; i < 12; i++) {
    jsonSchedule.add(i);
  }
  TEST_ASSERT_TRUE(dailySchedule.validateJSON(root));

  dailySchedule.updateFromJSON(root);
  TEST_ASSERT_EQUAL_STRING(dailySchedule._name, "foo3");
  for (uint8_t i = 0; i < 12; i++) {
    TEST_ASSERT_EQUAL(dailySchedule._schedule[i], i);
  }
}

void testDailyScheduleToJson() {
  const uint8_t schedule[] = {
    0xde, 0xad, 0xbe, 0xef,
    0xca, 0xfe, 0x13, 0x37,
    0x12, 0x34, 0x56, 0x78,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);

  StaticJsonDocument<CAPACITY_DAILY_SCHEDULE> doc;
  JsonObject root = doc.to<JsonObject>();
  TEST_ASSERT_TRUE(dailySchedule.toJSON(root));
  TEST_ASSERT_EQUAL_STRING(dailySchedule._name, root["name"]);
  for (uint8_t i = 0; i < 12; i++) {
    TEST_ASSERT_EQUAL(dailySchedule._schedule[i], root["schedule"][i]);
  }
}

void testUserSettingsManagerEmpty() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  TEST_ASSERT_TRUE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerSetPointsValid() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonSetPoints = root.createNestedArray("setPoints");
  ThermiteSetPoint setPoints[] = {
    ThermiteSetPoint("way too cold", 10.0f),
    ThermiteSetPoint("winter min", 21.0f),
    ThermiteSetPoint("summer max", 26.0f),
    ThermiteSetPoint("way too hot", 30.0f)
  };
  for (int i = 0; i < 4; i++) {
    JsonObject jsonSetPoint = jsonSetPoints.createNestedObject();
    setPoints[i].toJSON(jsonSetPoint);
  }
  TEST_ASSERT_TRUE(userSettingsManager.validateJSON(root));

  userSettingsManager.updateFromJSON(root);
  for (int i = 0; i < 4; i++) {
    TEST_ASSERT_EQUAL_STRING(userSettingsManager._setPoints[i]._name, setPoints[i]._name);
    TEST_ASSERT_EQUAL(
      userSettingsManager._setPoints[i]._tempTarget,
      setPoints[i]._tempTarget
    );
  }
}

void testUserSettingsManagerSetPointsEmpty() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonSetPoints = root.createNestedArray("setPoints");
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerSetPointsTooShort() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonSetPoints = root.createNestedArray("setPoints");
  ThermiteSetPoint setPoints[] = {
    ThermiteSetPoint("way too cold", 10.0f),
    ThermiteSetPoint("winter min", 21.0f),
    ThermiteSetPoint("summer max", 26.0f),
  };
  for (int i = 0; i < 3; i++) {
    JsonObject jsonSetPoint = jsonSetPoints.createNestedObject();
    setPoints[i].toJSON(jsonSetPoint);
  }
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerSetPointsTooLong() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonSetPoints = root.createNestedArray("setPoints");
  ThermiteSetPoint setPoints[] = {
    ThermiteSetPoint("way too cold", 10.0f),
    ThermiteSetPoint("just right", 19.5f),
    ThermiteSetPoint("winter min", 21.0f),
    ThermiteSetPoint("summer max", 26.0f),
    ThermiteSetPoint("way too hot", 30.0f)
  };
  for (int i = 0; i < 5; i++) {
    JsonObject jsonSetPoint = jsonSetPoints.createNestedObject();
    setPoints[i].toJSON(jsonSetPoint);
  }
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerSetPointsNotArray() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["setPoints"] = 73;
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerDailySchedulesValid() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonDailySchedules = root.createNestedArray("dailySchedules");
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);
  for (int i = 0; i < 4; i++) {
    JsonObject jsonDailySchedule = jsonDailySchedules.createNestedObject();
    dailySchedule.toJSON(jsonDailySchedule);
  }
  TEST_ASSERT_TRUE(userSettingsManager.validateJSON(root));

  userSettingsManager.updateFromJSON(root);
  for (int i = 0; i < 4; i++) {
    TEST_ASSERT_EQUAL_STRING(userSettingsManager._dailySchedules[i]._name, "foo");
    for (int j = 0; j < 12; j++) {
      TEST_ASSERT_EQUAL(
        userSettingsManager._dailySchedules[i]._schedule[j],
        schedule[j]
      );
    }
  }
}

void testUserSettingsManagerDailySchedulesEmpty() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonDailySchedules = root.createNestedArray("dailySchedules");
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerDailySchedulesTooShort() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonDailySchedules = root.createNestedArray("dailySchedules");
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);
  for (int i = 0; i < 3; i++) {
    JsonObject jsonDailySchedule = jsonDailySchedules.createNestedObject();
    dailySchedule.toJSON(jsonDailySchedule);
  }
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerDailySchedulesTooLong() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray jsonDailySchedules = root.createNestedArray("dailySchedules");
  const uint8_t schedule[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  ThermiteDailySchedule dailySchedule("foo", schedule);
  for (int i = 0; i < 5; i++) {
    JsonObject jsonDailySchedule = jsonDailySchedules.createNestedObject();
    dailySchedule.toJSON(jsonDailySchedule);
  }
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerDailySchedulesNotArray() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["dailySchedules"] = true;
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerWeeklyScheduleValid() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["weeklySchedule"] = 0x2a02;
  TEST_ASSERT_TRUE(userSettingsManager.validateJSON(root));

  userSettingsManager.updateFromJSON(root);
  TEST_ASSERT_EQUAL(userSettingsManager._weeklySchedule, 0x2a02);
}

void testUserSettingsManagerWeeklyScheduleOverflow() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["weeklySchedule"] = 0x10000;
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerWeeklyScheduleTopBitsSet() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["weeklySchedule"] = 0x4000;
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerWeeklyScheduleTooLow() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["weeklySchedule"] = -1;
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerTempOverrideValid() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["tempOverride"] = 17.5f;
  TEST_ASSERT_TRUE(userSettingsManager.validateJSON(root));

  userSettingsManager.updateFromJSON(root);
  TEST_ASSERT_EQUAL(userSettingsManager._tempOverride, 17.5f);
}

void testUserSettingsManagerTempOverrideTooLow() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["tempOverride"] = -100.0f;
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerTempOverrideTooHigh() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["tempOverride"] = 100.0f;
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerOverrideValid() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["overrideStart"] = 1612242000ul;
  root["overrideEnd"] = 1612846800ul;
  TEST_ASSERT_TRUE(userSettingsManager.validateJSON(root));

  userSettingsManager.updateFromJSON(root);
  TEST_ASSERT_EQUAL(userSettingsManager._overrideStart, 1612242000ul);
  TEST_ASSERT_EQUAL(userSettingsManager._overrideEnd, 1612846800ul);
}

void testUserSettingsManagerOverrideStartZeroEndNonZero() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["overrideStart"] = 0ul;
  root["overrideEnd"] = 1612846800ul;
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerOverrideStartNonZeroEndZero() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["overrideStart"] = 1612242000ul;
  root["overrideEnd"] = 0ul;
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerOverrideStartAfterEnd() {
  ThermiteUserSettingsManager userSettingsManager;

  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  root["overrideStart"] = 1612846800ul;
  root["overrideEnd"] = 1612242000ul;
  TEST_ASSERT_FALSE(userSettingsManager.validateJSON(root));
}

void testUserSettingsManagerToJson() {
  ThermiteUserSettingsManager userSettingsManager;
  
  StaticJsonDocument<CAPACITY_USER_SETTINGS_MANAGER> doc;
  JsonObject root = doc.to<JsonObject>();
  TEST_ASSERT_TRUE(userSettingsManager.toJSON(root));
  for (int i = 0; i < 4; i++) {
    const JsonObject& jsonSetPoint = root["setPoints"][i];
    TEST_ASSERT_EQUAL_STRING(
      userSettingsManager._setPoints[i]._name,
      jsonSetPoint["name"]
    );
    TEST_ASSERT_EQUAL(
      userSettingsManager._setPoints[i]._tempTarget,
      jsonSetPoint["tempTarget"]
    );
  }
  for (int i = 0; i < 4; i++) {
    const JsonObject& jsonDailySchedule = root["dailySchedules"][i];
    TEST_ASSERT_EQUAL_STRING(
      userSettingsManager._dailySchedules[i]._name,
      jsonDailySchedule["name"]
    );
    for (int j = 0; j < 12; j++) {
      TEST_ASSERT_EQUAL(
        userSettingsManager._dailySchedules[i]._schedule[j],
        jsonDailySchedule["schedule"][j]
      );
    }
  }
  TEST_ASSERT_EQUAL(userSettingsManager._weeklySchedule, root["weeklySchedule"]);
  TEST_ASSERT_EQUAL(userSettingsManager._tempOverride, root["tempOverride"]);
  TEST_ASSERT_EQUAL(userSettingsManager._overrideStart, root["overrideStart"]);
  TEST_ASSERT_EQUAL(userSettingsManager._overrideEnd, root["overrideEnd"]);
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(testSetPointEmpty);
  RUN_TEST(testSetPointNameValid);
  RUN_TEST(testSetPointNameEmpty);
  RUN_TEST(testSetPointNameTooLong);
  RUN_TEST(testSetPointTempTargetValid);
  RUN_TEST(testSetPointTempTargetTooLow);
  RUN_TEST(testSetPointTempTargetTooHigh);
  RUN_TEST(testSetPointBothValid);
  RUN_TEST(testSetPointToJson);

  RUN_TEST(testDailyScheduleEmpty);
  RUN_TEST(testDailyScheduleNameValid);
  RUN_TEST(testDailyScheduleNameEmpty);
  RUN_TEST(testDailyScheduleNameTooLong);
  RUN_TEST(testDailyScheduleScheduleValid);
  RUN_TEST(testDailyScheduleScheduleEmpty);
  RUN_TEST(testDailyScheduleScheduleTooShort);
  RUN_TEST(testDailyScheduleScheduleTooLong);
  RUN_TEST(testDailyScheduleScheduleNotArray);
  RUN_TEST(testDailyScheduleScheduleValueTooLow);
  RUN_TEST(testDailyScheduleScheduleValueTooHigh);
  RUN_TEST(testDailyScheduleBothValid);
  RUN_TEST(testDailyScheduleToJson);

  RUN_TEST(testUserSettingsManagerEmpty);
  RUN_TEST(testUserSettingsManagerSetPointsValid);
  RUN_TEST(testUserSettingsManagerSetPointsEmpty);
  RUN_TEST(testUserSettingsManagerSetPointsTooShort);
  RUN_TEST(testUserSettingsManagerSetPointsTooLong);
  RUN_TEST(testUserSettingsManagerSetPointsNotArray);
  RUN_TEST(testUserSettingsManagerDailySchedulesValid);
  RUN_TEST(testUserSettingsManagerDailySchedulesEmpty);
  RUN_TEST(testUserSettingsManagerDailySchedulesTooShort);
  RUN_TEST(testUserSettingsManagerDailySchedulesTooLong);
  RUN_TEST(testUserSettingsManagerDailySchedulesNotArray);
  RUN_TEST(testUserSettingsManagerWeeklyScheduleValid);
  RUN_TEST(testUserSettingsManagerWeeklyScheduleOverflow);
  RUN_TEST(testUserSettingsManagerWeeklyScheduleTopBitsSet);
  RUN_TEST(testUserSettingsManagerWeeklyScheduleTooLow);
  RUN_TEST(testUserSettingsManagerTempOverrideValid);
  RUN_TEST(testUserSettingsManagerTempOverrideTooLow);
  RUN_TEST(testUserSettingsManagerTempOverrideTooHigh);
  RUN_TEST(testUserSettingsManagerOverrideValid);
  RUN_TEST(testUserSettingsManagerOverrideStartZeroEndNonZero);
  RUN_TEST(testUserSettingsManagerOverrideStartNonZeroEndZero);
  RUN_TEST(testUserSettingsManagerOverrideStartAfterEnd);
  RUN_TEST(testUserSettingsManagerToJson);

  UNITY_END();
}

void loop() {}

#endif