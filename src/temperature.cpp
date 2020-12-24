#include "../include/temperature.h"

#include "../include/print.h"

double readTemperature() {
  /*
    Measurements (fever thermometer):
     Temp. | Resistance
     ------------------
     36.0C |  6.14kΩ
     36.8C |  5.92kΩ
     39.5C |  5.31kΩ

    Equations:
     multisolve(
      [x + y * ln(6140) + z * ln(6140)^3 = 1/(36.0 + 273.15),
       x + y * ln(5920) + z * ln(5920)^3 = 1/(36.8 + 273.15),
       x + y * ln(5310) + z * ln(5310)^3 = 1/(39.5 + 273.15)],
      [x, y, z])
     ==> [−0.008338999116, 0.001879352335, −0.000007261618354]

    Test:
     R = 6.01kΩ
     T = 1 / (−0.008338999116 + 0.001879352335 * ln(6010) + −0.000007261618354 * ln(6010)^3) - 273.15
     T = 36.4

    Measurements (fridge:
     Temp. | Resistance
     ------------------
        8C | 649
       20C | 11.70kΩ
       21C | 11.95kΩ

    Equations:
     multisolve(
      [x + y * ln(50100) + z * ln(50100)^3 = 1/(-14 + 273.15),
       x + y * ln(22950) + z * ln(22950)^3 = 1/(7 + 273.15),
       x + y * ln(11950) + z * ln(11950)^3 = 1/(21 + 273.15)],
      [x, y, z])
     ==> [0.005609420938, −0.0004593340289, 0.000002540864791]

    Test:
     R = 6.01kΩ
     T = 1 / (0.005 609 420 938 + −0.000 459 334 028 9 * ln(6010) + 0.000 002 540 864 791 * ln(6010)^3) - 273.15
     T = 36.4
  */
  int const rawADC = analogRead(PIN_F6);
  constexpr long pad = 9960;
  double const resistance = rawADC * pad / 512 + 800;
  print(Serial, F("ADC: "), rawADC, F(", resistance: "), resistance);

  double const lnrt = log(resistance);
  double const kelvin = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * lnrt * lnrt)) * lnrt);
  double const celsius = kelvin - 273.15;  // Convert Kelvin to Celsius

  return celsius;
}