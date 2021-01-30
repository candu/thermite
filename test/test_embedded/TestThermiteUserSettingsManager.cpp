#include <unity.h>

#include "ThermiteUserSettingsManager.cpp"

const size_t CAPACITY_SET_POINT =
  JSON_OBJECT_SIZE(2) + JSON_STRING_SIZE(16);
const size_t CAPACITY_DAILY_SCHEDULE =
  JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(12) + JSON_STRING_SIZE(16);
const size_t CAPACITY_USER_SETTINGS_MANAGER =
  JSON_OBJECT_SIZE(6) + CAPACITY_SET_POINT * 4 + CAPACITY_DAILY_SCHEDULE * 4;

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

void testSetPoint() {
  testSetPointEmpty();
  testSetPointNameValid();
  testSetPointNameEmpty();
  testSetPointNameTooLong();
  testSetPointTempTargetValid();
  testSetPointTempTargetTooLow();
  testSetPointTempTargetTooHigh();
  testSetPointBothValid();
  testSetPointToJson();
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

void testDailySchedule() {
  testDailyScheduleEmpty();
  testDailyScheduleNameValid();
  testDailyScheduleNameEmpty();
  testDailyScheduleNameTooLong();
  testDailyScheduleScheduleValid();
  testDailyScheduleScheduleEmpty();
  testDailyScheduleScheduleTooShort();
  testDailyScheduleScheduleTooLong();
  testDailyScheduleScheduleNotArray();
  testDailyScheduleScheduleValueTooLow();
  testDailyScheduleScheduleValueTooHigh();
  testDailyScheduleBothValid();
  testDailyScheduleToJson();
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(testSetPoint);
  RUN_TEST(testDailySchedule);
  // RUN_TEST(testUserSettingsManager);
  UNITY_END();
}

void loop() {}