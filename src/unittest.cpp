#include "homectl/unittest.h"

#include <Arduino.h>

UnitTest const *UnitTest::registry_;

UnitTest::UnitTest() : next_(registry_) { registry_ = this; }

void UnitTest::run() {
  Serial.begin(9600);
  while (!Serial) {
  }

  int testCount = 0;
  int testFailures = 0;
  for (UnitTest const *test = registry_; test != nullptr; test = test->next_) {
    Result ctx = test->context();
    Serial.print(ctx.fileName);
    Serial.print(':');
    Serial.print(ctx.lineNumber);
    Serial.print(':');
    Serial.print(ctx.functionName);
    Serial.print(':');
    test->run(ctx);
    testCount++;
    if (ctx.pass) {
      Serial.println("PASS");
    } else {
      testFailures++;
    }
  }

  Serial.println(F("-----------------------"));
  Serial.print(testCount);
  Serial.print(F(" Tests "));
  Serial.print(testFailures);
  Serial.print(F(" Failures "));
  Serial.println(F("0 Ignored "));
}
