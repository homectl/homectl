#include "../include/matrix.h"

#include <Arduino.h>

#include "../include/print.h"

void printMatrix(Stream &out, int rows, int cols, double const *data) {
  out << "Matrix<" << rows << ", " << cols << ">{{\n";
  for (int i = 0; i < rows; ++i) {
    out << "  {";
    for (int j = 0; j < cols; ++j) {
      out << data[i * cols + j];
      if (j != cols - 1) {
        out << ", ";
      }
    }
    out << "},\n";
  }
  out << "}}";
}

void printLinearFunction(Stream &out, int vars, double const *data) {
  out << data[0];
  for (int i = 0; i < vars; ++i) {
    out << " + " << data[i + 1] << "x" << (i + 1);
  }
}

void testMatrixTranspose() {
  constexpr Matrix<3, 2> m = {{
      {1, 2},
      {2, 3},
      {3, 4},
  }};

  static_assert(m.transpose() == Matrix<2, 3>{{
                                     {1, 2, 3},
                                     {2, 3, 4},
                                 }},
                "Transpose failed");
}

void testMatrixMult() {
  constexpr Matrix<2, 3> m1 = {{
      {3, -2, 5},
      {3, 0, 4},
  }};
  constexpr Matrix<3, 2> m2 = {{
      {2, 3},
      {-9, 0},
      {0, 4},
  }};
  static_assert(m1 * m2 == Matrix<2, 2>{{
                               {24, 29},
                               {6, 25},
                           }},
                "Matrix multiplication failed");
}

void testMatrixInverse2x2() {
  constexpr Matrix<2, 2> X{{
      {4, 7},
      {2, 6},
  }};
  constexpr Matrix<2, 2> X_inv{{
      {0.6, -0.7},
      {-0.2, 0.4},
  }};
  static_assert(inverse(X) == X_inv, "Inverse function failed");
}

void testMatrixInverse3x3() {
  constexpr Matrix<3, 3> X{{
      {5, 7, 9},
      {4, 3, 8},
      {7, 5, 6},
  }};
  constexpr Matrix<3, 3> X_inv{{
      {-0.21, 0.03, 0.28},
      {0.30, -0.31, -0.04},
      {-0.01, 0.23, -0.12},
  }};
  constexpr auto inv = inverse(X);
  static_assert(roundBy(inv, 100) == X_inv, "Inverse function failed");
}

void testMatrixSlice() {
  constexpr Matrix<3, 3> X{{
      {5, 7, 9},
      {4, 3, 8},
      {7, 5, 6},
  }};
  static_assert(X.slice<0, 0, 2, 2>() == Matrix<2, 2>{{
                                             {5, 7},
                                             {4, 3},
                                         }},
                "slice() failed");
  static_assert(X.slice<1, 0, 2, 2>() == Matrix<2, 2>{{
                                             {4, 3},
                                             {7, 5},
                                         }},
                "slice() failed");
  static_assert(X.slice<1, 1, 2, 2>() == Matrix<2, 2>{{
                                             {3, 8},
                                             {5, 6},
                                         }},
                "slice() failed");
  static_assert(X.slice<0, 0, 3, 1>() == Matrix<3, 1>{{
                                             {5},
                                             {4},
                                             {7},
                                         }},
                "slice() failed");
}

void testOLS2Vars() {
  constexpr Matrix<5, 2> X{{
      {1, 455},
      {1, 553},
      {1, 673},
      {1, 728},
      {1, 855},
  }};
  constexpr Matrix<5, 1> y{{
      {491},
      {663},
      {945},
      {1043},
      {1320},
  }};
  constexpr Matrix<2, 1> beta = ordinaryLeastSquares<2, 5>(X, y);
  static_assert(roundBy(beta, 100) == Matrix<2, 1>{{{-474.88}, {2.09}}});
}

void testSimpleLinearFunction() {
  // y = 1.25x - 0.5
  constexpr LinearFunction<1> f{{
      {1, 1},
      {2, 1.5},
      {3, 3.5},
  }};
  static_assert(roundBy(f.beta, 100) == Matrix<2, 1>{{{-0.5}, {1.25}}});
}

void testSimple2VarLinearFunction() {
  // y = -1300 + -50 x1 + 0 x2
  constexpr LinearFunction<2> f{{
      {15, 20, 400},
      {7, 17, 300},
      {2, 15, 200},
  }};
  static_assert(roundBy(f.beta, 100) ==
                Matrix<3, 1>{{{-4146.88}, {-101.10}, {303.27}}});
}

void testLinearFunction() {
  constexpr LinearFunction<1> f({{
      {455, 491},
      {553, 663},
      {673, 945},
      {728, 1043},
      {855, 1320},
  }});
  static_assert(roundBy(f.beta, 100) == Matrix<2, 1>{{{-474.88}, {2.09}}});
  static_assert(roundBy(f(0), 100) == -474.88, "");
}
