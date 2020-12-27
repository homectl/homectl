#include "../include/linreg.h"

#include "../include/print.h"

namespace LinearRegression {

Stream &operator<<(Stream &out, LinearFunction const &f) {
  return out << "y = " << f.f.m << F("x + ") << f.f.b << F(", r=") << f.f.r;
}

void testLinearFunction() {
  // y = 1.25x - 0.5
  constexpr LinearFunction f{{
      {0, 1, 1},
      {0, 2, 1.5},
      {0, 3, 3.5},
  }};
  static_assert(f.ok(), "Singular matrix error");
  static_assert(f(0) == -0.5, "Unexpected zero-point");
  static_assert(f(1) == 0.75, "Unexpected slope");
}

void testCatchZeroDenom() {
  // We can't draw a line because the points have no distance.
  constexpr LinearFunction f{{
      {0, 1, 1},
      {0, 1, 1},
  }};
  static_assert(!f.ok(), "Didn't catch zero division");
}

}  // namespace LinearRegression
