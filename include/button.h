#pragma once

#include <stdint.h>

class PushButton {
  uint8_t const pin_;
  bool switched_;
  int prevState_;

 public:
  explicit PushButton(uint8_t pin);

  void loop();
  bool isPushed() const { return switched_; }
};
