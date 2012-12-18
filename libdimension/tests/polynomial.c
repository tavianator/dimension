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
static const double poly[6] = {
  [5] =  2.0,
  [4] = -215.1602,
  [3] =  1540.4520864,
  [2] = -2430.5727856,
  [1] = -1292.541872,
  [0] =  2895.6432,
};

static double roots[5];
static size_t nroots;

#define DMNSN_CLOSE_ENOUGH 1.0e-6

DMNSN_TEST_SETUP(polynomial)
{
  nroots = dmnsn_polynomial_solve(poly, 5, roots);
}

DMNSN_TEST(polynomial, finds_positive_roots)
{
  ck_assert_int_eq(nroots, 4);
}

DMNSN_TEST(polynomial, local_min_roots)
{
  for (size_t i = 0; i < nroots; ++i) {
    double evmin = dmnsn_polynomial_evaluate(poly, 5, roots[i] - dmnsn_epsilon);
    double ev    = dmnsn_polynomial_evaluate(poly, 5, roots[i]);
    double evmax = dmnsn_polynomial_evaluate(poly, 5, roots[i] + dmnsn_epsilon);
    ck_assert(fabs(ev) < fabs(evmin) && fabs(ev) < fabs(evmax));
  }
}

DMNSN_TEST(polynomial, accurate_roots)
{
  for (size_t i = 0; i < nroots; ++i) {
    double ev = dmnsn_polynomial_evaluate(poly, 5, roots[i]);
    ck_assert(fabs(ev) < DMNSN_CLOSE_ENOUGH);
  }
}
