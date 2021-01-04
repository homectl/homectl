#pragma once

#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#include "homectl/AnalogWrite.h"
#include "homectl/Blink.h"
#include "homectl/Button.h"
#include "homectl/CO2.h"
#include "homectl/Matrix.h"
#include "homectl/Print.h"
#include "homectl/UsbEcho.h"

class Homectl {
  enum Pins {
#ifdef TEENSY
    LED = PIN_C4,
    BUTTON = PIN_C0,
#else
    LED = 2,
    BUTTON = 4,
#endif
  };

  struct State {
    CO2 co2{
#ifdef TEENSY
        Serial1
#else
        Serial2
#endif
    };
    Blink blink{Pins::LED};
    PushButton button{
        button.pushed.listen<Blink, &Blink::setEnabled>(blink),
        Pins::BUTTON,
    };
    LiquidCrystal_I2C lcd{0x27, 20, 4};
    UsbEcho usbEcho;

    unsigned long lastTime = millis();
    unsigned long iterations = 0;
  };

  State state;

  void handleLoopTimer();

 public:
  void setup();
  void loop();
};
