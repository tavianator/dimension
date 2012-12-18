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

/* poly[] = 2*(x + 1)*(x - 1.2345)*(x - 2.3456)*(x - 5)*(x - 100) */
const double poly[6] = {
  [5] =  2.0,
  [4] = -215.1602,
  [3] =  1540.4520864,
  [2] = -2430.5727856,
  [1] = -1292.541872,
  [0] =  2895.6432,
};

DMNSN_TEST("polynomial", finds_positive_roots)
{
  double x[5];
  size_t n = dmnsn_polynomial_solve(poly, 5, x);
  ck_assert_int_eq(n, 4);
}
DMNSN_END_TEST

DMNSN_TEST("polynomial", accurate_roots)
{
  double x[5];
  size_t n = dmnsn_polynomial_solve(poly, 5, x);

  for (size_t i = 0; i < n; ++i) {
    double evmin = dmnsn_polynomial_evaluate(poly, 5, x[i] - dmnsn_epsilon);
    double ev    = dmnsn_polynomial_evaluate(poly, 5, x[i]);
    double evmax = dmnsn_polynomial_evaluate(poly, 5, x[i] + dmnsn_epsilon);
    ck_assert(fabs(ev) < fabs(evmin) && fabs(ev) < fabs(evmax));
  }
}
DMNSN_END_TEST
