#pragma once

#include <Arduino.h>

/**
 * In production mode, disable USB debugging and enable any power saving we can
 * do. In non-production mode, more debug information is available.
 */
constexpr bool PRODUCTION = false;

/**
 * In debug mode, print extra status information to Serial. In unit tests,
 * Serial debugging is disabled because the test runner uses Serial for its
 * communication.
 */
#ifndef UNIT_TEST
constexpr bool DEBUG = true;  // Not unit test.
#else
constexpr bool DEBUG = false;
#endif

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

template <bool Enabled>
class Logger : public Print {
 private:
  Print *log_;

 public:
  Logger(Print &log, __FlashStringHelper const *file, int line,
         char const *func);
  Logger() : log_(nullptr) {}
  Logger(Logger &&other) : log_(other.log_) { other.log_ = nullptr; }
  ~Logger() { write('\n'); }
  size_t write(uint8_t b) override { return log_ ? log_->write(b) : 0; }
  size_t write(const uint8_t *buffer, size_t size) override {
    return log_ ? log_->write(buffer, size) : 0;
  };
};

template <>
class Logger<false> : public Print {
 public:
  Logger(Print &log, __FlashStringHelper const *file, int line,
         char const *func) {}
  size_t write(uint8_t b) override { return 0; }
};

static inline void doPrint(Print &out) {}

template <typename Arg, typename... Args>
void doPrint(Print &out, Arg const &arg, Args &&... args) {
  if (!DEBUG) return;
  out << arg;
  doPrint(out, args...);
}

template <typename... Args>
void doLog(__FlashStringHelper const *file, int line, char const *func,
           Args &&... args) {
  Logger<DEBUG> logger(Serial, file, line, func);
  doPrint(logger, args...);
}

template <typename... Args>
void doLogf(__FlashStringHelper const *file, int line, char const *func,
            Args &&... args) {
  Logger<DEBUG> logger(Serial, file, line, func);
  logger.printf(args...);
}

#define LOGGER(NAME) Logger<DEBUG> NAME(Serial, F(__FILE__), __LINE__, __func__)

#define LOG(...)                                             \
  do {                                                       \
    if (DEBUG) {                                             \
      doLog(F(__FILE__), __LINE__, __func__, ##__VA_ARGS__); \
    }                                                        \
  } while (0)
#define LOGF(...)                                           \
  do {                                                      \
    if (DEBUG) {                                            \
      doLogf(F(__FILE__), __LINE__, __func__, __VA_ARGS__); \
    }                                                       \
  } while (0)
