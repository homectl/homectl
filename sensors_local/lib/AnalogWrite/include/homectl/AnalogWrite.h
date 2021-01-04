#pragma once

#include <Arduino.h>

#ifndef TEENSY
typedef struct analog_write_channel {
  int8_t pin;
  double frequency;
  uint8_t resolution;
} analog_write_channel_t;

int analogWriteChannel(uint8_t pin);

void analogWriteFrequency(double frequency);
void analogWriteFrequency(uint8_t pin, double frequency);

void analogWriteResolution(uint8_t resolution);
void analogWriteResolution(uint8_t pin, uint8_t resolution);

void analogWrite(uint8_t pin, uint32_t value, uint32_t valueMax = 255);
#endif
