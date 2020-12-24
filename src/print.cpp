#include "../include/print.h"

void printTime(Stream &out, unsigned long currTime) {
  unsigned long const curr_min = currTime / 1000 / 60;
  unsigned long const curr_secs = currTime / 1000 % 60;
  unsigned long const curr_millis = currTime % 1000;
  out.printf(F("[%ld:%02ld.%03ld] "), curr_min, curr_secs, curr_millis);
}