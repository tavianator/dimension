/*************************************************************************
 * Copyright (C) 2010-2012 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * The Dimension Test Suite is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License as *
 * published by the Free Software Foundation; either version 3 of the    *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Test Suite is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

/**
 * @file
 * Basic tests of the polynomial root-finder.
 */

#include "tests.h"
#include "../polynomial.c"
#include <stdarg.h>

#define DMNSN_CLOSE_ENOUGH 1.0e-6

static void
dmnsn_assert_roots(const double poly[], size_t degree, size_t nroots_ex, ...)
{
  double roots[degree - 1];
  size_t nroots = dmnsn_polynomial_solve(poly, degree, roots);
  ck_assert_int_eq(nroots, nroots_ex);

  va_list ap;
  va_start(ap, nroots_ex);
  for (size_t i = 0; i < nroots; ++i) {
    double root_ex = va_arg(ap, double);
    bool found = false;
    for (size_t j = 0; j < nroots; ++j) {
      if (fabs(root_ex - roots[j]) >= dmnsn_epsilon) {
        continue;
      }
      found = true;

      double evroot = dmnsn_polynomial_evaluate(poly, degree, roots[j]);
      ck_assert(fabs(evroot) < DMNSN_CLOSE_ENOUGH);

      double evmin = dmnsn_polynomial_evaluate(poly, degree, roots[j] - dmnsn_epsilon);
      double evmax = dmnsn_polynomial_evaluate(poly, degree, roots[j] + dmnsn_epsilon);
      ck_assert(fabs(evroot) <= fabs(evmin) && fabs(evroot) <= fabs(evmax));

      break;
    }

    if (!found) {
      for (size_t j = 0; j < nroots; ++j) {
        fprintf(stderr, "roots[%zu] == %.17g\n", j, roots[j]);
      }
      ck_abort_msg("Expected root %.17g not found", root_ex);
    }
  }
  va_end(ap);
}


DMNSN_TEST(linear, no_positive_roots)
{
  // poly[] = x + 1
  static const double poly[] = {
    [1] = 1.0,
    [0] = 1.0,
  };
  dmnsn_assert_roots(poly, 1, 0);
}

DMNSN_TEST(linear, one_root)
{
  // poly[] = x - 1
  static const double poly[] = {
    [1] =  1.0,
    [0] = -1.0,
  };
  dmnsn_assert_roots(poly, 1, 1, 1.0);
}


DMNSN_TEST(quadratic, no_roots)
{
  // poly[] = x^2 + 1
  static const double poly[] = {
    [2] = 1.0,
    [1] = 0.0,
    [0] = 1.0,
  };
  dmnsn_assert_roots(poly, 2, 0);
}

DMNSN_TEST(quadratic, no_positive_roots)
{
  // poly[] = (x + 1)^2
  static const double poly[] = {
    [2] = 1.0,
    [1] = 2.0,
    [0] = 1.0,
  };
  dmnsn_assert_roots(poly, 2, 0);
}

DMNSN_TEST(quadratic, one_positive_root)
{
  // poly[] = (x + 1)*(x - 1)
  static const double poly[] = {
    [2] =  1.0,
    [1] =  0.0,
    [0] = -1.0,
  };
  dmnsn_assert_roots(poly, 2, 1, 1.0);
}

DMNSN_TEST(quadratic, two_roots)
{
  // poly[] = (x - 1.2345)*(x - 2.3456)
  static const double poly[] = {
    [2] =  1.0,
    [1] = -3.5801,
    [0] =  2.8956432,
  };
  dmnsn_assert_roots(poly, 2, 2, 1.2345, 2.3456);
}


DMNSN_TEST(cubic, no_positive_roots)
{
  // poly[] = x^3 + 1
  static const double poly[] = {
    [3] = 1.0,
    [2] = 0.0,
    [1] = 0.0,
    [0] = 1.0,
  };
  dmnsn_assert_roots(poly, 3, 0);
}

DMNSN_TEST(cubic, one_root)
{
  // poly[] = x^3 - 1
  static const double poly[] = {
    [3] =  1.0,
    [2] =  0.0,
    [1] =  0.0,
    [0] = -1.0,
  };
  dmnsn_assert_roots(poly, 3, 1, 1.0);
}

DMNSN_TEST(cubic, two_roots)
{
  // poly[] = (x - 1)*(x - 4)^2
  static const double poly[] = {
    [3] =   1.0,
    [2] =  -9.0,
    [1] =  24.0,
    [0] = -16.0,
  };
  dmnsn_assert_roots(poly, 3, 2, 1.0, 4.0);
}

DMNSN_TEST(cubic, three_roots)
{
  // poly[] = (x - 1.2345)*(x - 2.3456)*(x - 100)
  static const double poly[] = {
    [3] =    1.0,
    [2] = -103.5801,
    [1] =  360.9056432,
    [0] = -289.56432,
  };
  dmnsn_assert_roots(poly, 3, 3, 1.2345, 2.3456, 100.0);
}


DMNSN_TEST(quintic, four_roots)
{
  // poly[] = 2*(x + 1)*(x - 1.2345)*(x - 2.3456)*(x - 5)*(x - 100)
  static const double poly[] = {
    [5] =     2.0,
    [4] =  -215.1602,
    [3] =  1540.4520864,
    [2] = -2430.5727856,
    [1] = -1292.541872,
    [0] =  2895.6432,
  };
  dmnsn_assert_roots(poly, 5, 4, 1.2345, 2.3456, 5.0, 100.0);
}

// repeated_root[] = (x - 1)^6
static const double repeated_root[7] = {
  [6] =   1.0,
  [5] =  -6.0,
  [4] =  15.0,
  [3] = -20.0,
  [2] =  15.0,
  [1] =  -6.0,
  [0] =   1.0,
};

DMNSN_TEST(stability, equal_bounds)
{
  double root = dmnsn_bisect_root(repeated_root, 6, 1.0, 1.0);
  ck_assert_msg(root == 1.0, "root == %.17g", root);
}

DMNSN_TEST(stability, equal_values_at_bounds)
{
  double root = dmnsn_bisect_root(repeated_root, 6, 0.9, 1.1);
  ck_assert_msg(fabs(root - 1.0) < dmnsn_epsilon, "root == %.17g", root);
}
