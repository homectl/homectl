#include "../include/button.h"

#include "../include/print.h"

PushButton::PushButton(uint8_t pin)
    : pin_(pin), switched_(false), prevState_(HIGH) {
  // initialize the pushbutton pin as an input:
  pinMode(pin_, INPUT);
}

void PushButton::loop() {
  // read the state of the pushbutton value:
  int const buttonState = digitalRead(pin_);
  if (prevState_ != buttonState && buttonState == HIGH) {
    print(Serial, F("[button]: Button switched"));
    switched_ = !switched_;
  }
  prevState_ = buttonState;
}
