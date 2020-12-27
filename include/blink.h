#pragma once

#include <stdint.h>

/**
 * A slightly more sophisticated LED blinker using a PWM pin to make a soft
 * blink rather than a strong on/off.
 *
 * This blinker bounces between 0 and 255 on the PWM to slowly increase/decrease
 * the intensity of the LED.
 */
class Blink {
  bool enabled_ = true;
  uint8_t const pin_;
  uint8_t increment_ = 1;
  uint8_t value_ = 255;

 public:
  /**
   * Initialise a blinker on a given PWM pin.
   */
  explicit Blink(int pin);

  /**
   * Turn the blinker on (true) or off (false).
   */
  void enable(bool enabled);

  /**
   * Run one iteration of the blink loop, incrementing or decrementing the PWM
   * value on the LED pin.
   */
  void loop();
};
