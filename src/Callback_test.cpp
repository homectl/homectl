#include "homectl/Callback.h"

#include "homectl/unittest.h"

class TestListener {
 public:
  bool action(bool value) { return !value; }
};

TEST(Callback, Invoke) {
  Callback<bool(bool)> cb;
  TestListener ob{};
  cb.listen<TestListener, &TestListener::action>(ob);

  EXPECT_EQ(cb(true), false);
  EXPECT_EQ(cb(false), true);
}
