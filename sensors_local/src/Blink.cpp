#include "homectl/Blink.h"

#include <analogWrite.h>

Blink::Blink(uint8_t pin) : pin_(pin) {
  // initialize the digital pin as an output.
  pinMode(pin_, OUTPUT);
}

void Blink::setEnabled(bool enabled) { enabled_ = enabled; }

void Blink::loop() {
  if (!enabled_) {
    // turn LED off:
    digitalWrite(pin_, LOW);
    return;
  }

  // turn LED on:
  analogWrite(pin_, value_);

  value_ += increment_;
  if (value_ == 255) {
    increment_ = -1;
  }
  if (value_ == 0) {
    increment_ = 1;
  }
}
