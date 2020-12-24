#include "../include/power.h"

#include <Arduino.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void powerdown() {
  sbi(SMCR, SE);   // sleep enable, power down mode
  cbi(SMCR, SM0);  // power down mode
  sbi(SMCR, SM1);  // power down mode
  cbi(SMCR, SM2);  // power down mode
  asm volatile("sleep");
}