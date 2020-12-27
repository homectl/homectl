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
    print(Serial, F("I received: "), recv);
  }
}

void handleLoopTimer() {
  unsigned long const currTime = millis();

  if (currTime - state.lastTime >= secsPerLog * 1000) {
    print(Serial, Time(currTime), state.iterations / secsPerLog,
          F(" iterations per second ("),
          secsPerLog / double(state.iterations) * 1e6, 0,
          F("μs per iteration)"));
    state.lastTime = currTime / 1000 * 1000;
    state.iterations = 0;

    CO2::Reading co2Reading = state.co2.read();
    print(Serial, co2Reading);
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
  print(Serial, F("Setup starting"));
  // Disable USB, reduces power consumption by around 11mA.
  if (PRODUCTION) {
    Serial.end();
  }

  // print(Serial, F("Sending calibration command to CO2 sensor"));
  // state.co2.calibrateZeroPoint();
  // state.co2.setABC(false);
  // state.co2.calibrateSpanPoint(1000);

  // Running the CPU at 1MHz should now consume around 6mA.

  EICRA |= (1 << ISC01);  // Trigger on falling edge
  EIMSK |= (1 << INT0);   // Enable external interrupt INT0
  sei();                  // Enable global interrupts

  print(Serial, Time(millis()), ": Setup complete");
}

#endif  // UNIT_TEST

ISR(INT0_vect) {
  print(Serial, __func__, " D0=", analogRead(PIN_D0),
        " D1=", analogRead(PIN_D1));
}
