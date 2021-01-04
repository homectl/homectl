#include "../include/Callback.h"

#include "unittest.h"

class TestListener {
 public:
  bool action(bool value) { return !value; }
};

TEST(Callback, Invoke) {
  Callback<bool(bool)> cb;
  TestListener ob{};
  connect(cb, TestListener, action, ob);

  EXPECT_EQ(cb(true), false);
  EXPECT_EQ(cb(false), true);
}

UNITTEST_MAIN()
