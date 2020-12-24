#pragma once

#include <math.h>

struct Stream;

namespace LinearRegression {

struct Point {
  double x;
  double y;
};

static constexpr double square(double x) { return x * x; }

class Sums {
  static constexpr double sumx(int n, Point const points[]) {
    return n == 0 ? 0 : points[n - 1].x + sumx(n - 1, points);
  }
  static constexpr double sumx2(int n, Point const points[]) {
    return n == 0 ? 0 : square(points[n - 1].x) + sumx2(n - 1, points);
  }
  static constexpr double sumy(int n, Point const points[]) {
    return n == 0 ? 0 : points[n - 1].y + sumy(n - 1, points);
  }
  static constexpr double sumy2(int n, Point const points[]) {
    return n == 0 ? 0 : square(points[n - 1].y) + sumy2(n - 1, points);
  }
  static constexpr double sumxy(int n, Point const points[]) {
    return n == 0 ? 0
                  : points[n - 1].x * points[n - 1].y + sumxy(n - 1, points);
  }

 public:
  double x;
  double x2;
  double y;
  double y2;
  double xy;

  constexpr Sums(int n, Point const points[])
      : x(sumx(n, points)),
        x2(sumx2(n, points)),
        y(sumy(n, points)),
        y2(sumy2(n, points)),
        xy(sumxy(n, points)) {}
};

class LinearFunction {
  struct Data {
    bool ok;
    double m;
    double b;
    /* correlation coeff */
    double r;
  };

  static constexpr double sqrtNewtonRaphson(double x, double curr,
                                            double prev) {
    return curr == prev ? curr
                        : sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
  }

  static constexpr double sqrt(double x) {
    return x < 0 || x >= INFINITY ? NAN : sqrtNewtonRaphson(x, x, 0);
  }

  static constexpr Data compute1(int n, Sums const &sums, double denom) {
    return {
        /* ok */ true,
        /* m */ (n * sums.xy - sums.x * sums.y) / denom,
        /* b */ (sums.y * sums.x2 - sums.x * sums.xy) / denom,
        /* r */ (sums.xy - sums.x * sums.y / n) /
            sqrt((sums.x2 - square(sums.x) / n) *
                 (sums.y2 - square(sums.y) / n)),
    };
  }

  static constexpr Data decide(int n, Sums const &sums, double denom) {
    // If denominator is 0, we have a singular matrix => can't solve the
    // problem.
    return denom == 0 ? Data() : compute1(n, sums, denom);
  }

  static constexpr Data compute(int n, Sums const &sums) {
    return decide(n, sums, n * sums.x2 - square(sums.x));
  }

  Data f;

 public:
  template <int N>
  constexpr LinearFunction(Point const (&points)[N])
      : f(compute(N, Sums(N, points))) {}

  constexpr double operator()(double x) const { return f.m * x + f.b; }
  constexpr bool ok() const { return f.ok; }

  friend Stream &operator<<(Stream &out, LinearFunction const &f);
};

}  // namespace LinearRegression