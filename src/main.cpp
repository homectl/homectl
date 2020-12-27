#include <Arduino.h>

#include "../include/CO2.h"
#include "../include/blink.h"
#include "../include/print.h"

/**
 * In production mode, disable USB debugging and enable any power saving we can
 * do. In non-production mode, more debug information is available.
 */
constexpr bool PRODUCTION = false;
/**
 * Number of seconds to wait between logging sensor measurements. This is
 * currently 5, because the MH-Z19B doesn't do more than one measurement per 5
 * seconds.
 */
constexpr int secsPerLog = 1;

constexpr int ledPin = PIN_C4;
constexpr int buttonPin = PIN_C7;

static struct State {
  CO2 co2{Serial1};
  Blink blink{ledPin};
  int prev_state = HIGH;
  bool switched = true;

  unsigned long lastTime = millis();
  unsigned long iterations = 0;
} state;

void handleButton() {
  // read the state of the pushbutton value:
  int const button_state = digitalRead(buttonPin);
  if (state.prev_state != button_state && button_state == HIGH) {
    Serial.println(F("Button switched"));
    state.switched = !state.switched;
  }
  state.prev_state = button_state;
}

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
    printTime(Serial, currTime);
    print(Serial, state.iterations / secsPerLog, F(" iterations per second ("),
          secsPerLog / double(state.iterations) * 1e6, 0,
          F("μs per iteration)"));
    state.lastTime = currTime / 1000 * 1000;
    state.iterations = 0;

    // int photo_value = analogRead(PIN_F7);
    // float celsius = readTemperature();
    CO2::Reading co2Reading = state.co2.read();

    print(Serial,
          // F("Photoresistor: "), photo_value,
          // F(", Thermistor: "), celsius,
          F("CO2: "), co2Reading.ppm_raw, F(", Corrected: "),
          co2Reading.ppm_corrected, F(", Temp (CO2): "), co2Reading.temperature,
          F(", Unknown: "), co2Reading.unknown);
  }

  ++state.iterations;
}

#ifndef UNIT_TEST

// the setup routine runs once when you press reset:
void setup() {
#if 0
  // Disable input on all pins, reduces power consumption by around 3mA.
  for (int i = 0; i < 46; ++i) {
    pinMode(i, OUTPUT);
  }
#endif

  Serial.begin(9600);
  Serial.println(F("Setup starting"));
  // Disable USB, reduces power consumption by around 11mA.
  if (PRODUCTION) {
    Serial.end();
  }

  // print(Serial, F("Sending calibration command to CO2 sensor"));
  // state.co2.calibrateZeroPoint();
  // state.co2.setABC(false);
  // state.co2.calibrateSpanPoint(1000);

  // Running the CPU at 1MHz should now consume around 6mA.

  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);

  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  pinMode(PIN_F7, INPUT);
  pinMode(PIN_F6, INPUT);

  EICRA |= (1 << ISC01);  // Trigger on falling edge
  EIMSK |= (1 << INT0);   // Enable external interrupt INT0
  sei();                  // Enable global interrupts
  pinMode(PIN_C0, INPUT);

  printTime(Serial, millis());
  Serial.println(F("Setup complete"));

  delay(2000);

  double data[][3] = {
      {13, 540, 571},  {13, 563, 631},

      {14, 580, 676},

      {15, 482, 413},  {15, 571, 561}, {15, 582, 663}, {15, 621, 726},
      {15, 631, 704},  {15, 686, 859}, {15, 704, 891},

      {16, 621, 752},  {16, 600, 670}, {16, 631, 696}, {16, 644, 751},
      {16, 723, 933},

      {17, 609, 755},  {17, 618, 767}, {17, 637, 834}, {17, 642, 838},
      {17, 734, 1009},

      {18, 565, 681},

      {20, 530, 630},

      {21, 553, 723},
  };

  for (auto const &row : data) {
    print(Serial, row[0], ' ', row[1], ' ', row[2]);
  }
}

void loop() {
  // handleButton();
  state.blink.loop();
  handleUsb();
  handleLoopTimer();
}

#endif  // UNIT_TEST

// ISR(INT0_vect) { Serial.println(__func__); }
