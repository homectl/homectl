#pragma once

#include <Arduino.h>

class Time : public Printable {
 public:
  unsigned long const currTime;
  explicit Time(unsigned long currTime) : currTime(currTime) {}

  size_t printTo(Print &out) const override;
};

class Bytes : public Printable {
  byte const *data_;
  size_t size_;

 public:
  template <int N>
  explicit Bytes(byte const (&data)[N]) : Bytes(data, N) {}

  Bytes(byte const *data, size_t size) : data_(data), size_(size) {}

  size_t printTo(Print &out) const override {
    size_t ret = 0;
    // print out the response in hex
    for (size_t i = 0; i < size_; i++) {
      ret += out.printf(" %02X", data_[i]);
    }
    return ret;
  }
};

template <typename Arg>
Print &operator<<(Print &out, Arg const &arg) {
  out.print(arg);
  return out;
}

static inline Print &operator<<(Print &out, __FlashStringHelper const *arg) {
  out.print(String(arg));
  return out;
}
