#include "../include/blink.h"

#include <Arduino.h>

Blink::Blink(int pin) : pin_(pin) {
  // initialize the digital pin as an output.
  pinMode(pin_, OUTPUT);
}

void Blink::enable(bool enabled) { enabled_ = enabled; }

void Blink::loop() {
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (enabled_) {
    // turn LED on:
    analogWrite(pin_, value_);
  } else {
    // turn LED off:
    digitalWrite(pin_, LOW);
  }

  value_ += increment_;
  if (value_ == 255) {
    increment_ = -1;
  }
  if (value_ == 0) {
    increment_ = 1;
  }
}
