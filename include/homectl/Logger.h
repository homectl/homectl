#pragma once

#include <Arduino.h>

#include "homectl/Print.h"
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

struct LoggerTraits {
  /**
   * The maximum length of a log line.
   */
  static constexpr int LINE_LEN = 128;
  /**
   * The maximum number of log lines this logger buffers.
   */
  static constexpr int BUFFER_LEN = 10;
  /**
   * Number of milliseconds to sleep between writing log lines to the output.
   */
  static constexpr int DELAY = 100;
};

/**
 * Thread-safe asynchronous logger.
 *
 * The logger allocates (LINE_LEN + sizeof(uint8_t *) * BUFFER_LEN) of static
 * memory up front, and then never resizes to avoid non-stack-like dynamic
 * allocations.
 *
 * The actual log writing happens in a separate thread, running on a separate
 * core (core 0), while the log formatting happens on the caller's core (core
 * 1). We perform the actual I/O on a separate thread since the most
 * time-consuming part is the synchronous output to USB.
 *
 * Besides LINE_LEN and BUFFER_LEN, the third configurable value is DELAY. This
 * influences how frequently we flush the log lines and write to the output.
 *
 * Overall, it depends on how much you're logging, how busy you want to make the
 * logger core, and how much memory you are willing to allocate to the logger.
 * If you're logging very long lines, keep in mind that the buffer will have
 * allocated BUFFER_LEN times the memory for the longest line.
 */
template <bool Enabled, typename TraitsT = LoggerTraits>
class Logger : public Print {
 private:
  using Traits = TraitsT;

  struct LogLine {
    uint8_t data[Traits::LINE_LEN];
    uint8_t *cur;

    bool empty() const { return cur == nullptr || cur == data; }

    /**
     * The constructor does not zero-initialise data, since we're never going to
     * read it anyway and zeroing out all those bytes takes a long time.
     */
    LogLine() : cur(data) {}
    LogLine(LogLine &&rhs) { *this = std::move(rhs); }
    LogLine &operator=(LogLine &&rhs) {
      // Only copy over the actually relevant bytes, leaving garbage in the
      // remaining bytes.
      memcpy(data, rhs.data, rhs.cur - rhs.data);
      cur = data + (rhs.cur - rhs.data);
      rhs.cur = rhs.data;
      return *this;
    }
  };

  using LogQueue = ThreadSafeQueue<LogLine, Traits::BUFFER_LEN>;
  static_assert(sizeof(LogQueue) == 1328,
                "unexpected memory size of log queue");

  LogLine line_;

  static Print *&output();
  static LogQueue &queue();
  static TaskHandle_t &task();

  static void writeLines(void *);

 public:
  Logger(__FlashStringHelper const *file, int line, char const *func,
         Time timestamp);
  Logger() {}
  Logger(Logger &&other) : line_(std::move(other.line_)) {}
  ~Logger() {
    if (!line_.empty()) {
      queue().add(std::move(line_));
    }
  }
  size_t write(uint8_t b) override {
    if (line_.cur == nullptr || line_.cur == line_.data + Traits::LINE_LEN) {
      return 0;
    }
    *line_.cur = b;
    ++line_.cur;
    return 1;
  }

  static void setOutput(Print &out);
  static void setup();
};

template <typename Traits>
class Logger<false, Traits> : public Print {
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
