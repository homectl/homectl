#include "homectl/Homectl.h"

/**
 * Number of seconds to wait between logging sensor measurements. This is
 * currently 6, because the MH-Z19B doesn't do more than one measurement per
 * 6 seconds.
 */
constexpr int secsPerLog = 6;

void Homectl::handleLoopTimer() {
  unsigned long const currTime = millis();

  if (currTime - state.lastTime >= secsPerLog * 1000) {
    LOG(state.iterations / secsPerLog, F(" iterations per second ("),
        secsPerLog / double(state.iterations) * 1e6, 0, F("μs per iteration)"));
    state.lastTime = currTime / 1000 * 1000;
    state.iterations = 0;

    CO2::Reading co2Reading = state.co2.read();
    LOG(co2Reading);
    state.lcd.clear();
    co2Reading.printTo(state.lcd, [](Print &out, int col, int row) {
      static_cast<LiquidCrystal_I2C &>(out).setCursor(col, row);
    });
  }

  ++state.iterations;
}

void Homectl::loop() {
  state.blink.loop();
  state.button.loop();
  state.usbEcho.loop();

  handleLoopTimer();

  delay(1);
}

// the setup routine runs once when you press reset:
void Homectl::setup() {
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

  // Switch on the backlight
  state.lcd.init();
  state.lcd.backlight();
  state.lcd.home();
  state.lcd.print("Welcome to Homectl");

  LOG("setup complete");
}
