#include <unity.h>

#include "ThermiteUserSettingsManager.cpp"

const size_t CAPACITY_SET_POINT = JSON_OBJECT_SIZE(2);

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

void setup() {
  UNITY_BEGIN();
  RUN_TEST(testSetPoint);
  UNITY_END();
}

void loop() {}