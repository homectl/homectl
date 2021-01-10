#pragma once

#include <stdint.h>

#include "homectl/Callback.h"

class PushButton {
  EV_OBJECT(PushButton)

 public:
  explicit PushButton(uint8_t pin);

  Callback<void(bool)> pushed;

 private:
  uint8_t const pin_;
  bool switched_;
  uint8_t prevState_;
};
