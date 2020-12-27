#pragma once

#include <stdint.h>

#include "../include/Callback.h"

class PushButton {
  uint8_t const pin_;
  bool switched_;
  uint8_t prevState_;

 public:
  explicit PushButton(uint8_t pin);

  void loop();
  bool isPushed() const { return switched_; }

  Callback<void(bool)> pushed;
};
