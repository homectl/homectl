#include "homectl/Print.h"

template <>
ThreadSafeQueue<Logger<true>::LogLine, Logger<true>::BUFFER_LEN>
    &Logger<true>::queue() {
  static ThreadSafeQueue<Logger<true>::LogLine, BUFFER_LEN> ob;
  return ob;
}

template <>
Print *&Logger<true>::output() {
  static Print *ob;
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
    if (output() != nullptr) {
      Serial.printf("Queue size: %d\n", queue().size());
      queue().consume([](LogLine const &line) {
        if (line.cur == nullptr) {
          return;
        }
        output()->write(line.data, line.cur - line.data);
        output()->println();
      });
    }

    delay(1000);
  }
}

template <>
void Logger<true>::setup() {
  output() = &Serial;
  xTaskCreatePinnedToCore(writeLines, /* Function to implement the task */
                          "Logger",   /* Name of the task */
                          10000,      /* Stack size in words */
                          NULL,       /* Task input parameter */
                          0,          /* Priority of the task */
                          &task(),    /* Task handle. */
                          0);         /* Core where the task should run */
}

size_t Time::printTo(Print &out) const {
  unsigned long const curr_min = currTime / 1000 / 60;
  unsigned long const curr_secs = currTime / 1000 % 60;
  unsigned long const curr_millis = currTime % 1000;
  return out.printf("[%ld:%02ld.%03ld]", curr_min, curr_secs, curr_millis);
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
  while (sz < maxLogPadding && sz < LINE_LEN) {
    sz += print(' ');
  }
}
