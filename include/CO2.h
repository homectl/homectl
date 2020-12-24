#pragma once

#include <Arduino.h>

/**
 * MH-Z19b sensor interface using a user-supplied UART stream.
 */
class CO2 {
  Stream &input_;
  unsigned long lastRequest_ = 0;

 public:
  struct Reading {
    int ppm_raw;
    int ppm_corrected;
    int temperature;
    int unknown;
  };

  explicit CO2(HardwareSerial &input);
  void setABC(bool enabled) const;
  void calibrateZeroPoint() const;
  void calibrateSpanPoint(uint16_t ppm) const;
  Reading read();
};