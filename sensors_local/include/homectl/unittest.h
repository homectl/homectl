#pragma once

#include <Arduino.h>

class UnitTest {
  static UnitTest const *registry_;

  UnitTest const *next_;

 protected:
  struct Result {
    __FlashStringHelper const *const className;
    __FlashStringHelper const *const functionName;
    __FlashStringHelper const *const fileName;
    int const lineNumber;
    bool pass = true;
  };

  UnitTest();

  virtual Result context() const = 0;
  virtual void run(Result &ctx) const = 0;

 public:
  static void run();
};

#ifdef UNIT_TEST
#define TEST_INSTANCE(CLASS, NAME) test##CLASS##_##NAME
#else
#define TEST_INSTANCE(CLASS, NAME)
#endif

#define TEST(CLASS, NAME)                 \
  class CLASS##_##NAME : UnitTest {       \
   public:                                \
    using UnitTest::UnitTest;             \
                                          \
    Result context() const override {     \
      return {                            \
          F(#CLASS),                      \
          F(#NAME),                       \
          F(__FILE__),                    \
          __LINE__,                       \
      };                                  \
    }                                     \
                                          \
    void run(Result &ctx) const override; \
  } TEST_INSTANCE(CLASS, NAME);           \
  void CLASS##_##NAME::run(Result &ctx) const

static inline void testPrint(Print &out) {}

template <typename Arg, typename... Args>
void testPrint(Print &out, Arg const &arg, Args &&... args) {
  out.print(arg);
  testPrint(out, args...);
}

template <typename... Args>
void testFail(Args const &... args) {
  Serial.print("FAIL:");
  testPrint(Serial, args...);
  Serial.println();
}

#define EXPECT_EQ(A, B)       \
  do {                        \
    if (A != B) {             \
      testFail(A, " != ", B); \
      ctx.pass = false;       \
    }                         \
  } while (0)

#define UNITTEST_MAIN()           \
  void setup() {}                 \
                                  \
  void loop() {                   \
    if (Serial && Serial.dtr()) { \
      UnitTest::run();            \
      Serial.clear();             \
    }                             \
    delay(100);                   \
  }
