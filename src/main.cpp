#include <Arduino.h>

#include "../include/CO2.h"
#include "../include/print.h"

static const bool PRODUCTION = false;

static const int ledPin = PIN_C4;
static const int buttonPin = PIN_C7;

static struct State {
  // the loop routine runs over and over again forever:
  uint8_t x = 255;
  uint8_t increment = 1;
  int prev_state = HIGH;
  bool switched = true;

  unsigned long lastTime = millis();
  unsigned long iterations = 0;
} state;

static CO2 co2(Serial1);

void handleButton() {
  // read the state of the pushbutton value:
  int const button_state = digitalRead(buttonPin);
  if (state.prev_state != button_state && button_state == HIGH) {
    Serial.println(F("Button switched"));
    state.switched = !state.switched;
  }
  state.prev_state = button_state;
}

void handleLed() {
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (state.switched) {
    // turn LED on:
    analogWrite(ledPin, state.x);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }

  state.x += state.increment;
  if (state.x == 255) {
    state.increment = -1;
  }
  if (state.x == 0) {
    state.increment = 1;
  }
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

  if (currTime - state.lastTime >= 1000) {
    printTime(Serial, currTime);
    print(Serial, state.iterations, F(" iterations per second ("),
          1 / double(state.iterations) * 1e6, 0, F("μs per iteration)"));
    state.lastTime = currTime / 1000 * 1000;
    state.iterations = 0;

    // int photo_value = analogRead(PIN_F7);
    // float celsius = readTemperature();
    CO2::Reading co2Reading = co2.read();

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
  // co2.calibrateZeroPoint();
  // co2.setABC(false);
  // co2.calibrateSpanPoint(1000);

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
}

void loop() {
  // handleButton();
  handleLed();
  handleUsb();
  handleLoopTimer();
}

#endif  // UNIT_TEST

ISR(INT0_vect) { Serial.println(__func__); }
