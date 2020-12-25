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

/**
 * This class implements the least squares regression method in constexpr.
 *
 * The code here is intended to run at compile time given a @c Point array, but
 * can also run at runtime.
 *
 * In the example, we compute a simple linear function based on 3 points.
 *
 * @code
 * // y = 1.25x - 0.5
 * constexpr LinearFunction f{{
 *   {1, 1},
 *   {2, 1.5},
 *   {3, 3.5},
 * }};
 * static_assert(f.ok(), "Singular matrix error");
 * static_assert(f(0) == -0.5, "Unexpected zero-point");
 * static_assert(f(1) == 0.75, "Unexpected slope");
 * @endcode
 *
 * See linreg.cpp for more examples in test cases.
 */
class LinearFunction {
  /**
   * Contains the actual function parameters. We store this in a separate struct
   * because we compute all the values at the same time. We can't use local
   * variables or assignments in the constructor, so the constructor must be a
   * single initialisation expression for the @c f field.
   */
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

  /**
   * We reimplement sqrt in constexpr even though @c __builtin_sqrt (and by
   * extension, the AVR @c sqrt function) is constexpr because by standard it is
   * not, and IDEs tend to not like us being non-standard.
   */
  static constexpr double sqrt(double x) {
    return x < 0 || x >= INFINITY ? NAN : sqrtNewtonRaphson(x, x, 0);
  }

  /**
   * The final step of computing the function parameters after being certain
   * that there won't be a division by 0 and after summing all the x, y, x^2,
   * y^2, and xy in the @c Sums class.
   */
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

  /**
   * Protect against division by 0. In case of zero-division, return an all-zero
   * @c Data object with @c ok set to @c false.
   */
  static constexpr Data decide(int n, Sums const &sums, double denom) {
    // If denominator is 0, we have a singular matrix => can't solve the
    // problem.
    return denom == 0 ? Data() : compute1(n, sums, denom);
  }

  /**
   * Main entry point for the computation after having computed all the sums.
   */
  static constexpr Data compute(int n, Sums const &sums) {
    return decide(n, sums, n * sums.x2 - square(sums.x));
  }

  /**
   * The function parameter object.
   */
  Data f;

 public:
  /**
   * Constructor computes a linear function based on the provided set of points.
   */
  template <int N>
  constexpr LinearFunction(Point const (&points)[N])
      : f(compute(N, Sums(N, points))) {}

  /**
   * Apply the linear function to a value.
   */
  constexpr double operator()(double x) const { return f.m * x + f.b; }
  /**
   * Returns the OK status of the computation. If false, the function is zero.
   */
  constexpr bool ok() const { return f.ok; }

  /**
   * Print the function parameters.
   */
  friend Stream &operator<<(Stream &out, LinearFunction const &f);
};

}  // namespace LinearRegression
