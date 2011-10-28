/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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

/*
 * Basic test of numerical polynomial root-finder
 */

#include "dimension.h"
#include <stddef.h>
#include <stdio.h>

int
main(void)
{
  /* Treat warnings as errors for tests */
  dmnsn_die_on_warnings(true);

  double poly[6], x[5];
  /* poly[] = 2*(x + 1)*(x - 1.2345)*(x - 2.3456)*(x - 5)*(x - 100) */
  poly[5] =  2.0;
  poly[4] = -215.1602;
  poly[3] =  1540.4520864;
  poly[2] = -2430.5727856;
  poly[1] = -1292.541872;
  poly[0] =  2895.6432;

  size_t n = dmnsn_polynomial_solve(poly, 5, x);
  if (n != 4) {
    fprintf(stderr,
            "--- Wrong number of roots found (%zu, should be %u) ---\n",
            n, 4);
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < n; ++i) {
    double evmin = dmnsn_polynomial_evaluate(poly, 5, x[i] - dmnsn_epsilon);
    double ev    = dmnsn_polynomial_evaluate(poly, 5, x[i]);
    double evmax = dmnsn_polynomial_evaluate(poly, 5, x[i] + dmnsn_epsilon);
    if (fabs(evmin) < fabs(ev) || fabs(evmax) < fabs(ev)) {
      fprintf(stderr, "--- Root %.15g is inaccurate! ---\n", x[i]);
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
