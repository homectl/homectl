#pragma once

#include <Arduino.h>

/**
 * In production mode, disable USB debugging and enable any power saving we can
 * do. In non-production mode, more debug information is available.
 */
constexpr bool PRODUCTION = false;
/**
 * In debug mode, print extra status information to Serial.
 */
constexpr bool DEBUG = true;

class Time : public Printable {
 public:
  unsigned long const currTime;
  explicit Time(unsigned long currTime) : currTime(currTime) {}

  size_t printTo(Print &out) const override;
};

class Bytes : public Printable {
  byte *data_;
  size_t size_;

 public:
  template <int N>
  explicit Bytes(byte (&data)[N]) : data_(data), size_(N) {}

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

static inline void print(Print &out) { out.println(); }

template <typename Arg, typename... Args>
void print(Print &out, Arg const &arg, Args &&... args) {
  if (!DEBUG) return;
  out << arg;
  print(out, args...);
}

template <typename... Args>
void printf(Print &out, Args &&... args) {
  if (!DEBUG) return;
  out.printf(args...);
}
