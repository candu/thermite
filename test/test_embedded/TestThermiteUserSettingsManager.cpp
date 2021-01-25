#ifdef UNIT_TEST

#include <unity.h>

#include "ThermiteUserSettingsManager.h"

void testSetPoint() {
  TEST_ASSERT_EQUAL(2 + 2, 4);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(testSetPoint);
  UNITY_END();
}

void loop() {}

#endif