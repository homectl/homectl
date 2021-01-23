#include "homectl/Homectl.h"

/**
 * Number of seconds to wait between logging sensor measurements. This is
 * currently 6, because the MH-Z19B doesn't do more than one measurement per
 * 6 seconds.
 */
constexpr int secsPerLog = 6;

static void pad(Print &out, size_t sz) {
  while (sz < Homectl::LCD_COLS) sz += out.print(' ');
}

void Homectl::State::showPMSReading(PMS5003T::Reading const &reading) {
  // We got a reading, so put the sensor back to sleep.
  pms5003t.sleep(true);

  lcd.setCursor(0, 3);
  pad(lcd, reading.printTo(lcd));
}

void Homectl::State::showCO2Reading(CO2::Reading const &reading) {
  LOG(reading);
  lcd.setCursor(0, 0);
  pad(lcd, reading.printTo(lcd));
}

void Homectl::handleLoopTimer() {
  unsigned long const currTime = millis();

  if (currTime - state.lastTime >= secsPerLog * 1000) {
    LOG(state.iterations / secsPerLog, F(" iterations per second ("),
        secsPerLog / double(state.iterations) * 1e6, 0, F("μs per iteration)"));
    state.lastTime = currTime / 1000 * 1000;
    state.iterations = 0;

    state.co2.requestReading();

    float temperature = state.dht.readTemperature(false);
    state.lcd.setCursor(0, 1);
    pad(state.lcd, state.lcd.printf("Temp: %.2fC", temperature));

    float humidity = state.dht.readHumidity();
    state.lcd.setCursor(0, 2);
    pad(state.lcd, state.lcd.printf("Hum: %.2f%%", humidity));
  }

  ++state.iterations;
}

void Homectl::loop() {
  state.blink.loop();
  state.button.loop();
  state.usbEcho.loop();
  state.pms5003t.loop();
  state.co2.loop();

  handleLoopTimer();

  delay(1);
}

// the setup routine runs once when you press reset:
void Homectl::setup() {
  Logger<DEBUG>::setup();

  state.dht.begin();

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

  // Switch on the LCD backlight, clear screen, and print welcome message.
  state.lcd.init();
  state.lcd.backlight();
  state.lcd.home();
  state.lcd.print("Welcome to Homectl");

  LOG("setup complete");
}
