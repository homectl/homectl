#include "../include/blink.h"

#include <Arduino.h>

Blink::Blink(int ledPin) : ledPin_(ledPin) {}

void Blink::enable(bool enabled) { enabled_ = enabled; }

void Blink::loop() {
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (enabled_) {
    // turn LED on:
    analogWrite(ledPin_, value_);
  } else {
    // turn LED off:
    digitalWrite(ledPin_, LOW);
  }

  value_ += increment_;
  if (value_ == 255) {
    increment_ = -1;
  }
  if (value_ == 0) {
    increment_ = 1;
  }
}