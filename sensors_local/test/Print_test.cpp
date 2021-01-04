#include "homectl/Print.h"

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

TEST(Print, ZeroTime) {
  StringPrint out;
  out << Time(0);
  EXPECT_EQ(out.str(), "[0:00.000]");
}

TEST(Print, HundredMinutes) {
  StringPrint out;
  out << Time(100UL * 60 * 1000 + 6987);
  EXPECT_EQ(out.str(), "[100:06.987]");
}

TEST(Print, Bytes) {
  StringPrint out;
  byte data[] = {0x3E, 0xAA, 0x00};
  out << Bytes(data);
  EXPECT_EQ(out.str(), " 3E AA 00");
}

TEST(Print, Logger) {
  StringPrint out;
  Logger<true> logger(out, F("file.cpp"), 123, "myfunc", Time(1234));
  doPrint(logger, "hello ", 123, ' ', 2.34);
  EXPECT_EQ(out.str(), "[0:01.234] file.cpp:123 (myfunc) hello 123 2.34");
}
