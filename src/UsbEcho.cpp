#include "homectl/UsbEcho.h"

#include <Arduino.h>

#include "homectl/Logger.h"

static bool isLineBreak(char c) { return c == '\r' || c == '\n'; }

static void skipLineBreaks(Stream &in) {
  while (in.available() > 0 && isLineBreak(in.peek())) {
    in.read();
  }
}

void UsbEcho::loop() {
  char const *const et = recv_ + sizeof recv_;

  // Read the incoming byte, if there is one.
  if (Serial.available() > 0) {
    *it_ = Serial.read();
    if (isLineBreak(*it_) || it_ + 1 == et) {
      skipLineBreaks(Serial);
      // Remove the line break character if there is one.
      it_[!isLineBreak(*it_)] = '\0';
      // Echo back the buffer.
      LOG(F("I received: '"), recv_, "'");
      it_ = recv_;
    } else {
      ++it_;
    }
  }
}
