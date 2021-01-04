#pragma once

// This code requires a new-ish version of GCC. Version 7.3.0 will do. If you
// use platformio, you may need to customise platformio.ini to specify a later
// version of toolchain-atmelavr. See the platformio.ini file in this repo for a
// known-good version.

template <int Rows, int Cols>
class Matrix {
 public:
  static_assert(Rows >= 1, "Matrix must have at least 1 row");
  static_assert(Cols >= 1, "Matrix must have at least 1 column");

  struct Row {
    double elts[Cols];

    constexpr double &operator[](int i) { return elts[i]; }
    constexpr double const &operator[](int i) const { return elts[i]; }
  };

  constexpr Matrix(Row const (&data)[Rows] = {}) : data(data) {}
  constexpr explicit Matrix(double x) : data{} {
    for (int i = 0; i < Rows; ++i) {
      for (int j = 0; j < Cols; ++j) {
        data[i][j] = x;
      }
    }
  }

  constexpr Row &operator[](int i) { return data[i]; }
  constexpr Row const &operator[](int i) const { return data[i]; }

  constexpr Matrix<Cols, Rows> transpose() const {
    Matrix<Cols, Rows> res;
    for (int i = 0; i < Rows; ++i) {
      for (int j = 0; j < Cols; ++j) {
        res[j][i] = data[i][j];
      }
    }
    return res;
  }

  template <int FromRow, int FromCol, int OutRows, int OutCols>
  constexpr Matrix<OutRows, OutCols> slice() const {
    Matrix<OutRows, OutCols> r{};
    for (int i = FromRow; i < FromRow + OutRows; ++i) {
      for (int j = FromCol; j < FromCol + OutCols; ++j) {
        r[i - FromRow][j - FromCol] = data[i][j];
      }
    }
    return r;
  }

 private:
  Row data[Rows];
};

template <int n>
constexpr Matrix<n, n> inverse(Matrix<n, n> const &m) {
  Matrix<n * 2, n * 2> a;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      a[i][j] = m[i][j];
    }
  }

  // Initialising right-hand side to identity matrix.
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < 2 * n; ++j) {
      if (j == i + n) {
        a[i][j] = 1;
      }
    }
  }

  // Partial pivoting.
  for (int i = n; i > 1; --i) {
    if (a[i - 1][1] < a[i][1]) {
      for (int j = 0; j < 2 * n; ++j) {
        double const d = a[i][j];
        a[i][j] = a[i - 1][j];
        a[i - 1][j] = d;
      }
    }
  }

  // Reducing to diagonal matrix.
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < 2 * n; ++j) {
      if (j != i) {
        double const d = a[j][i] / a[i][i];
        for (int k = 0; k < n * 2; ++k) {
          a[j][k] -= a[i][k] * d;
        }
      }
    }
  }

  // Reducing to unit matrix.
  for (int i = 0; i < n; ++i) {
    double const d = a[i][i];
    for (int j = 0; j < 2 * n; ++j) {
      a[i][j] = a[i][j] / d;
    }
  }

  // Result is in the right hand side.
  Matrix<n, n> r{};
  for (int i = 0; i < n; ++i) {
    for (int j = n; j < 2 * n; ++j) {
      r[i][j - n] = a[i][j];
    }
  }

  return r;
}

template <int Rows, int Cols>
constexpr bool operator==(Matrix<Rows, Cols> const &lhs,
                          Matrix<Rows, Cols> const &rhs) {
  for (int i = 0; i < Rows; ++i) {
    for (int j = 0; j < Cols; ++j) {
      if (lhs[i][j] != rhs[i][j]) {
        return false;
      }
    }
  }
  return true;
}

template <int RowsL, int ColsL, int RowsR, int ColsR>
constexpr Matrix<RowsL, ColsR> operator*(Matrix<RowsL, ColsL> const &lhs,
                                         Matrix<RowsR, ColsR> const &rhs) {
  static_assert(ColsL == RowsR,
                "number of columns of first matrix must equal number of rows "
                "of second matrix");
  Matrix<RowsL, ColsR> mult{};
  for (int i = 0; i < RowsL; ++i) {
    for (int j = 0; j < ColsR; ++j) {
      for (int k = 0; k < ColsL; ++k) {
        mult[i][j] += lhs[i][k] * rhs[k][j];
      }
    }
  }
  return mult;
}

template <int Rows, int ColsL, int ColsR>
constexpr Matrix<Rows, ColsL + ColsR> hconcat(Matrix<Rows, ColsL> const &lhs,
                                              Matrix<Rows, ColsR> const &rhs) {
  Matrix<Rows, ColsL + ColsR> r;
  for (int i = 0; i < Rows; ++i) {
    for (int j = 0; j < ColsL; ++j) {
      r[i][j] = lhs[i][j];
    }
    for (int j = 0; j < ColsR; ++j) {
      r[i][j + ColsL] = rhs[i][j];
    }
  }
  return r;
}

template <int Vars, int Eqs>
constexpr Matrix<Vars, 1> ordinaryLeastSquares(Matrix<Eqs, Vars> const &X,
                                               Matrix<Eqs, 1> const &y) {
  return inverse(X.transpose() * X) * X.transpose() * y;
}

template <int Vars>
class LinearFunction {
  using Beta = Matrix<Vars + 1, 1>;

  template <int Eqs>
  static constexpr Beta compute(Matrix<Eqs, Vars + 1> const &m) {
    auto const &X = m.template slice<0, 0, Eqs, Vars>();
    auto const &y = m.template slice<0, Vars, Eqs, 1>();
    return ordinaryLeastSquares(hconcat(Matrix<Eqs, 1>(1), X), y);
  }

 public:
  Beta beta;

  template <int Eqs>
  constexpr LinearFunction(
      typename Matrix<Eqs, Vars + 1>::Row const (&eqs)[Eqs])
      : beta(compute(Matrix<Eqs, Vars + 1>(eqs))) {}

  template <typename... Args>
  constexpr double operator()(Args... args) const {
    static_assert(sizeof...(args) == Vars,
                  "need exactly one function argument per variable");
    Matrix<1, 1> const r = Matrix<1, Vars + 1>{{{1, double(args)...}}} * beta;
    return r[0][0];
  }
};

static constexpr double roundToZero(double x) {
  return x >= 0 ? (long)(x + 0.5) : (long)(x - 0.5);
}

static constexpr double roundBy(double x, double multiplier) {
  return roundToZero(x * multiplier) / multiplier;
}

template <int Rows, int Cols>
constexpr Matrix<Rows, Cols> roundBy(Matrix<Rows, Cols> const &m,
                                     int multiplier) {
  Matrix<Rows, Cols> res = m;
  for (int i = 0; i < Rows; ++i) {
    for (int j = 0; j < Cols; ++j) {
      res[i][j] = roundBy(res[i][j], multiplier);
    }
  }
  return res;
}

struct Print;

void printMatrix(Print &out, int rows, int cols, double const *data);

template <int Rows, int Cols>
Print &operator<<(Print &out, Matrix<Rows, Cols> const &m) {
  printMatrix(out, Rows, Cols, &m[0][0]);
  return out;
}

void printLinearFunction(Print &out, int vars, double const *data);

template <int Vars>
Print &operator<<(Print &out, LinearFunction<Vars> const &f) {
  printLinearFunction(out, Vars, &f.beta[0][0]);
  return out;
}
