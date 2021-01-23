#include "homectl/Button.h"

#include "homectl/Logger.h"

PushButton::PushButton(uint8_t pin)
    : pin_(pin), switched_(true), prevState_(HIGH) {
  // initialize the pushbutton pin as an input:
  pinMode(pin_, INPUT);
}

void PushButton::loop() {
  // read the state of the pushbutton value:
  uint8_t const buttonState = digitalRead(pin_);
  if (prevState_ != buttonState && buttonState == HIGH) {
    LOG(F("button switched"));
    switched_ = !switched_;
    pushed(switched_);
  }
  prevState_ = buttonState;
}
