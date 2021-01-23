#include "homectl/Logger.h"

template <>
Logger<true>::LogQueue &Logger<true>::queue() {
  static LogQueue ob;
  return ob;
}

template <>
Print *&Logger<true>::output() {
  static Print *ob = &Serial;
  return ob;
}

template <>
void Logger<true>::setOutput(Print &out) {
  output() = &out;
}

template <>
TaskHandle_t &Logger<true>::task() {
  static TaskHandle_t ob;
  return ob;
}

template <>
void Logger<true>::writeLines(void *) {
  while (true) {
    delay(Traits::DELAY);
    if (output() == nullptr) {
      continue;
    }

    int const queueSize = queue().size();
    if (queueSize == Traits::BUFFER_LEN) {
      Serial.println(
          F("WARNING: Logger queue was full; you may have lost log lines"));
    }
    for (LogLine const &line : queue().consume()) {
      if (line.empty()) {
        break;
      }
      output()->write(line.data, line.cur - line.data);
      output()->println();
    }
  }
}

template <>
void Logger<true>::setup() {
  // 1024 bytes ought to be enough for anybody (turns out, 640 is not).
  //
  // These 1024 bytes cover the stack requirements of writeLines locals and all
  // the HardwareSerial/USB stuff happening below it. We copy the queue into a
  // temporary, so we need to cover for that separately.
  constexpr uint32_t STACK_SIZE = sizeof(queue()) + 1024;

  xTaskCreatePinnedToCore(writeLines, /* Function to implement the task */
                          "Logger",   /* Name of the task */
                          STACK_SIZE, /* Stack size in bytes */
                          NULL,       /* Task input parameter */
                          0,          /* Priority of the task */
                          &task(),    /* Task handle. */
                          0);         /* Core where the task should run */
}

template <>
Logger<true>::Logger(__FlashStringHelper const *file, int line,
                     char const *func, Time timestamp) {
  String fileName(file);
  int idx = fileName.lastIndexOf('/');
  if (idx == -1) idx = fileName.lastIndexOf('\\');
  if (idx != -1) fileName = fileName.substring(idx + 1);

  size_t sz = 0;
  sz += print(timestamp);
  sz += print(' ');
  sz += print(fileName);
  sz += print(':');
  sz += print(line);
  sz += print(F(" ("));
  sz += print(func);
  sz += print(')');
  static size_t maxLogPadding = 0;
  if (sz >= maxLogPadding) {
    maxLogPadding = sz + 1;
  }
  while (sz < maxLogPadding && sz < Traits::LINE_LEN) {
    sz += print(' ');
  }
}
