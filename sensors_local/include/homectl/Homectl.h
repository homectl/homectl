#pragma once

#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <analogWrite.h>

#include "homectl/Blink.h"
#include "homectl/Button.h"
#include "homectl/CO2.h"
#include "homectl/Matrix.h"
#include "homectl/PMS5003T.h"
#include "homectl/Print.h"
#include "homectl/UsbEcho.h"

class Homectl {
  struct Pins {
    enum {
      LED = 2,
      BUTTON = 4,
      DHT = 5,
    };
  };

  struct State {
    CO2 co2{Serial2};
    Blink blink{Pins::LED};
    PushButton button{
        button.pushed.listen<Blink, &Blink::setEnabled>(blink),
        Pins::BUTTON,
    };
    LiquidCrystal_I2C lcd{0x27, LCD_COLS, LCD_ROWS};
    UsbEcho usbEcho;
    DHT dht{Pins::DHT, DHT22};
    PMS5003T pms5003t{
        pms5003t.newReading.listen<State, &State::showPMSReading>(*this),
        Serial1,
    };

    unsigned long lastTime = millis();
    unsigned long iterations = 0;

    void showPMSReading(PMS5003T::Reading const &reading);
  };

  State state;

  void handleLoopTimer();

 public:
  static constexpr uint8_t LCD_COLS = 20;
  static constexpr uint8_t LCD_ROWS = 4;

  void setup();
  void loop();
};
