#include "homectl/Print.h"

size_t Time::printTo(Print &out) const {
  unsigned long const curr_min = currTime / 1000 / 60;
  unsigned long const curr_secs = currTime / 1000 % 60;
  unsigned long const curr_millis = currTime % 1000;
  return out.printf("[%ld:%02ld.%03ld]", curr_min, curr_secs, curr_millis);
}
