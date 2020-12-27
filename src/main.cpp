#include <Arduino.h>

#include "../include/CO2.h"
#include "../include/blink.h"
#include "../include/print.h"

/**
 * Number of seconds to wait between logging sensor measurements. This is
 * currently 6, because the MH-Z19B doesn't do more than one measurement per 6
 * seconds.
 */
constexpr int secsPerLog = 6;

static struct State {
  CO2 co2{Serial1};
  Blink blink{PIN_C4};

  unsigned long lastTime = millis();
  unsigned long iterations = 0;
} state;

void handleUsb() {
  char recv[100];
  char *it = recv;
  char *et = recv + sizeof recv;
  while (Serial.available() > 0 && it + 1 != et) {
    // read the incoming byte:
    *it = Serial.read();
    ++it;
  }
  *it = '\0';

  if (recv[0] != '\0') {
    // say what you got:
    LOG(F("I received: "), recv);
  }
}

void handleLoopTimer() {
  unsigned long const currTime = millis();

  if (currTime - state.lastTime >= secsPerLog * 1000) {
    LOG(state.iterations / secsPerLog, F(" iterations per second ("),
        secsPerLog / double(state.iterations) * 1e6, 0, F("μs per iteration)"));
    state.lastTime = currTime / 1000 * 1000;
    state.iterations = 0;

    CO2::Reading co2Reading = state.co2.read();
    LOG(co2Reading);
  }

  ++state.iterations;
}

#ifndef UNIT_TEST

void loop() {
  state.blink.loop();
  handleUsb();
  handleLoopTimer();
}

// the setup routine runs once when you press reset:
void setup() {
  if (PRODUCTION) {
    // Disable input on all pins, reduces power consumption by around 3mA.
    for (int i = 0; i < 46; ++i) {
      pinMode(i, OUTPUT);
    }
  }

  if (DEBUG) {
    Serial.begin(9600);
  }
  LOG(F("setup starting"));
  // Disable USB, reduces power consumption by around 11mA.
  if (PRODUCTION) {
    Serial.end();
  }

  // LOG(F("Sending calibration command to CO2 sensor"));
  // state.co2.calibrateZeroPoint();
  // state.co2.setABC(false);
  // state.co2.calibrateSpanPoint(1000);

  EICRA |= (1 << ISC01);  // Trigger on falling edge
  EIMSK |= (1 << INT0);   // Enable external interrupt INT0
  sei();                  // Enable global interrupts

  LOG("setup complete");
}

#endif  // UNIT_TEST

ISR(INT0_vect) { LOG(" D0=", analogRead(PIN_D0), " D1=", analogRead(PIN_D1)); }
