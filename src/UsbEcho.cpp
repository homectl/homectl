#include "../include/UsbEcho.h"

#include <Arduino.h>

#include "../include/Print.h"

void UsbEcho::loop() {
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
