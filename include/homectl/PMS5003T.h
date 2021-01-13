#pragma once

#include <Arduino.h>

#include "homectl/Callback.h"

class PMS5003T {
  EV_OBJECT(PMS5003T)

 public:
  static constexpr uint8_t RX_PIN = 32;
  static constexpr uint8_t TX_PIN = 33;

  struct Reading {
    uint16_t pm1_0_std = -1;
    uint16_t pm2_5_std = -1;
    uint16_t pm10_std = -1;
    uint16_t pm1_0_atm = -1;
    uint16_t pm2_5_atm = -1;
    uint16_t pm10_atm = -1;
    uint16_t pm0_3_cnt = -1;
    uint16_t pm0_5_cnt = -1;
    uint16_t pm1_0_cnt = -1;
    uint16_t pm2_5_cnt = -1;
    uint16_t temp = -1;
    uint16_t hum = -1;
    uint16_t data13 = -1;
    uint16_t checksum = -1;

    static uint16_t readU16(byte const *payload) {
      return uint16_t(payload[0]) << 8 | payload[1];
    }

    Reading() {}

    explicit Reading(byte const (&payload)[28])
        : pm1_0_std(readU16(&payload[0])),
          pm2_5_std(readU16(&payload[2])),
          pm10_std(readU16(&payload[4])),
          pm1_0_atm(readU16(&payload[6])),
          pm2_5_atm(readU16(&payload[8])),
          pm10_atm(readU16(&payload[10])),
          pm0_3_cnt(readU16(&payload[12])),
          pm0_5_cnt(readU16(&payload[14])),
          pm1_0_cnt(readU16(&payload[16])),
          pm2_5_cnt(readU16(&payload[18])),
          temp(readU16(&payload[20])),
          hum(readU16(&payload[22])),
          data13(readU16(&payload[24])),
          checksum(readU16(&payload[26])) {}

    size_t printTo(Print &out) const;
  };

  explicit PMS5003T(HardwareSerial &io);
  void sleep(bool enabled);
  Reading const &read() const;

  Callback<void(Reading const &)> newReading;

 private:
  HardwareSerial io_;
  Reading reading_;
};
