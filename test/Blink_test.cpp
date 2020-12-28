#include "../include/Blink.h"

#include "unittest.h"

TEST(PinNumber, BuiltinLedNumber) {
  // Works for Teensy++ 2.0
  EXPECT_EQ(LED_BUILTIN, 6);
}
