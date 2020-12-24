#include "../include/linreg.h"

#include "../include/print.h"

namespace LinearRegression {

Stream &operator<<(Stream &out, LinearFunction const &f) {
  return out << f.f.m << F(", b=") << f.f.b << F(", r=") << f.f.r;
}

}