#pragma once

struct LinearFunction {
  double m;
  double b;
  /* correlation coeff */
  double r;

  constexpr double operator()(double x) const { return m * x + b; }
};

namespace LinearRegression {

struct Point {
  double x;
  double y;
};

constexpr double square(double x) { return x * x; }

constexpr double sqrtNewtonRaphson(double x, double curr, double prev) {
  return curr == prev ? curr
                      : sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
}

constexpr double sqrt(double x) {
  return x < 0 || x >= INFINITY ? NAN : sqrtNewtonRaphson(x, x, 0);
}

class Sums {
  constexpr double sumx(int n, Point const points[]) {
    return n == 0 ? 0 : points[n - 1].x + sumx(n - 1, points);
  }
  constexpr double sumx2(int n, Point const points[]) {
    return n == 0 ? 0 : square(points[n - 1].x) + sumx2(n - 1, points);
  }
  constexpr double sumy(int n, Point const points[]) {
    return n == 0 ? 0 : points[n - 1].y + sumy(n - 1, points);
  }
  constexpr double sumy2(int n, Point const points[]) {
    return n == 0 ? 0 : square(points[n - 1].y) + sumy2(n - 1, points);
  }
  constexpr double sumxy(int n, Point const points[]) {
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

struct Result {
  bool ok;
  LinearFunction f;

  static constexpr Result success(double m, double b, double r) {
    return {true, {m, b, r}};
  }

  static constexpr Result fail() { return {false}; }
};

constexpr Result compute1(int n, Sums const &sums, double denom) {
  return Result::success(
      /* m */ (n * sums.xy - sums.x * sums.y) / denom,
      /* b */ (sums.y * sums.x2 - sums.x * sums.xy) / denom,
      /* r */ (sums.xy - sums.x * sums.y / n) /
          sqrt((sums.x2 - square(sums.x) / n) *
               (sums.y2 - square(sums.y) / n)));
}

constexpr Result decide(int n, Sums const &sums, double denom) {
  // If denominator is 0, we have a singular matrix => can't solve the problem.
  return denom == 0 ? Result::fail() : compute1(n, sums, denom);
}

constexpr Result compute(int n, Sums const &sums) {
  return decide(n, sums, n * sums.x2 - square(sums.x));
}

class Line {
  Result result_;

 public:
  template <int N>
  constexpr Line(Point const (&points)[N])
      : result_(compute(N, Sums(N, points))) {}

  constexpr operator LinearFunction() const { return result_.f; }
};

}  // namespace LinearRegression