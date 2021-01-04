#pragma once

#include <stdint.h>

#include "homectl/Callback.h"

class PushButton {
  uint8_t const pin_;
  bool switched_;
  uint8_t prevState_;

 public:
  template <typename Registry, typename... Rest>
  PushButton(EventRegistered<Registry> evt, Rest const &... rest)
      : PushButton(rest...) {
    evt.listen();
  }

  explicit PushButton(uint8_t pin);

  void loop();

  struct Pushed : Callback<void(bool)> {
  } pushed;
};
