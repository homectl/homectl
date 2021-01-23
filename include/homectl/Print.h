#pragma once

#include <Arduino.h>

#include "homectl/Queue.h"

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

template <bool Enabled>
class Logger : public Print {
 private:
  static constexpr int BUFFER_LEN = 16;
  static constexpr int LINE_LEN = 128;

  struct LogLine {
    uint8_t data[LINE_LEN];
    uint8_t *cur;

    LogLine() : cur(data) {}
    LogLine(LogLine &&rhs) { *this = std::move(rhs); }
    LogLine &operator=(LogLine &&rhs) {
      memcpy(data, rhs.data, rhs.cur - rhs.data);
      cur = data + (rhs.cur - rhs.data);
      rhs.cur = rhs.data;
      return *this;
    }
  };

  LogLine line_;

  static Print *&output();
  static ThreadSafeQueue<LogLine, BUFFER_LEN> &queue();
  static TaskHandle_t &task();

  static void writeLines(void *);

 public:
  Logger(__FlashStringHelper const *file, int line, char const *func,
         Time timestamp);
  Logger() {}
  Logger(Logger &&other) : line_(std::move(other.line_)) {}
  ~Logger() {
    if (line_.cur != nullptr) {
      queue().add(std::move(line_));
    }
  }
  size_t write(uint8_t b) override {
    if (line_.cur == nullptr || line_.cur == line_.data + LINE_LEN) return 0;
    *line_.cur = b;
    ++line_.cur;
    return 1;
  }

  static void setOutput(Print &out);
  static void setup();
};

template <>
class Logger<false> : public Print {
 public:
  Logger() {}
  Logger(__FlashStringHelper const *file, int line, char const *func,
         Time timestamp) {}
  size_t write(uint8_t b) override { return 0; }

  static void setup() {}
};

static inline void doPrint(Print &out) {}

template <typename Arg, typename... Args>
void doPrint(Print &out, Arg const &arg, Args &&...args) {
  out << arg;
  doPrint(out, args...);
}

template <typename... Args>
Logger<DEBUG> doLog(__FlashStringHelper const *file, int line, char const *func,
                    Args &&...args) {
  Logger<DEBUG> logger(file, line, func, Time(millis()));
  doPrint(logger, args...);
  return logger;
}

template <typename... Args>
Logger<DEBUG> doLogf(__FlashStringHelper const *file, int line,
                     char const *func, __FlashStringHelper const *fmt,
                     Args &&...args) {
  Logger<DEBUG> logger(file, line, func, Time(millis()));
#ifdef TEENSY
  logger.printf(fmt, args...);
#else
  logger.printf(String(fmt).begin(), args...);
#endif
  return logger;
}

static inline Logger<DEBUG> noLog() { return {}; }

#define LOG(...) \
  (DEBUG ? doLog(F(__FILE__), __LINE__, __func__, ##__VA_ARGS__) : noLog())
#define LOGF(...) \
  (DEBUG ? doLogf(F(__FILE__), __LINE__, __func__, __VA_ARGS__) : noLog())
