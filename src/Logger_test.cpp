#include "homectl/Logger.h"

#include "homectl/unittest.h"

class StringPrint : public Print {
  String str_;

 public:
  size_t write(uint8_t b) override {
    str_ += char(b);
    return 1;
  }

  String const &str() const { return str_; }
};

TEST(Print, Logger) {
  Logger<true> logger(F("file.cpp"), 123, "myfunc", Time(1234));
  doPrint(logger, "hello ", 123, ' ', 2.34);
  StringPrint out;
  Logger<true>::setOutput(out);
  EXPECT_EQ(out.str(), "[0:01.234] file.cpp:123 (myfunc) hello 123 2.34");
  Logger<true>::setOutput(Serial);
}
