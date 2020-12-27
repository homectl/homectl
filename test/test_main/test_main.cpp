#include <Arduino.h>
#include <unity.h>

void test_led_builtin_pin_number() {
  TEST_ASSERT_EQUAL(LED_BUILTIN, 6);
  TEST_ASSERT_EQUAL(LED_BUILTIN, 6);
}

void test_led_builtin_pin_number_wrong() {
  TEST_ASSERT_EQUAL(LED_BUILTIN, 7);
  TEST_ASSERT_EQUAL(LED_BUILTIN, 8);
}

void setup() {
  delay(2000);  // for Arduino framework
  while (!Serial) {
  }

  UNITY_BEGIN();
  RUN_TEST(test_led_builtin_pin_number);
  RUN_TEST(test_led_builtin_pin_number_wrong);
  UNITY_END();
}

void loop() {}
