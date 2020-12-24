#include <Arduino.h>
#include <unity.h>

#ifdef UNIT_TEST

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void test_led_builtin_pin_number() {
  TEST_ASSERT_EQUAL(LED_BUILTIN, 6);
  TEST_ASSERT_EQUAL(LED_BUILTIN, 6);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_led_builtin_pin_number);
}

void loop() {
  UNITY_END();
  /*
  cbi(SMCR, SE);   // sleep enable, power down mode
  cbi(SMCR, SM0);  // power down mode
  sbi(SMCR, SM1);  // power down mode
  cbi(SMCR, SM2);  // power down mode
  */
}

#endif  // UNIT_TEST