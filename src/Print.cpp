#include "homectl/Print.h"

size_t Time::printTo(Print &out) const {
  unsigned long const curr_min = currTime / 1000 / 60;
  unsigned long const curr_secs = currTime / 1000 % 60;
  unsigned long const curr_millis = currTime % 1000;
  return out.printf("[%ld:%02ld.%03ld]", curr_min, curr_secs, curr_millis);
}

template <>
Logger<true>::Logger(Print &log, __FlashStringHelper const *file, int line,
                     char const *func, Time timestamp)
    : log_(&log) {
  String fileName(file);
  int idx = fileName.lastIndexOf('/');
  if (idx == -1) idx = fileName.lastIndexOf('\\');
  if (idx != -1) fileName = fileName.substring(idx + 1);

  size_t sz = 0;
  sz += log_->print(timestamp);
  sz += log_->print(' ');
  sz += log_->print(fileName);
  sz += log_->print(':');
  sz += log_->print(line);
  sz += log_->print(F(" ("));
  sz += log_->print(func);
  sz += log_->print(')');
  static size_t maxLogPadding = 0;
  if (sz >= maxLogPadding) {
    maxLogPadding = sz + 1;
  }
  while (sz < maxLogPadding) {
    sz += log_->print(' ');
  }
}