/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Benchmark Suite.                   *
 *                                                                       *
 * The Dimension Benchmark Suite is free software; you can redistribute  *
 * it and/or modify it under the terms of the GNU General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Benchmark Suite is distributed in the hope that it will *
 * be useful, but WITHOUT ANY WARRANTY; without even the implied         *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See *
 * the GNU General Public License for more details.                      *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#define DMNSN_INLINE extern inline
#include "../math/polynomial.c"
#include <sandglass.h>
#include <stdlib.h>

int
main(void)
{
#define NPOLY 5
  double p[NPOLY][NPOLY + 1], x[NPOLY];

  // p[0][] = x - 0.5;
  p[0][1] =  1.0;
  p[0][0] = -0.5;

  // p[1][] = (x + 0.5)*(x - 0.5)
  p[1][2] =  1.0;
  p[1][1] =  0.0;
  p[1][0] = -0.25;

  // p[2][] = (x + 1)*(x - 1.2345)*(x - 100)
  p[2][3] =  1.0;
  p[2][2] = -100.2345;
  p[2][1] =  22.2155;
  p[2][0] =  123.45;

  // p[3][] = (x + 1)*(x - 1.2345)*(x - 5)*(x - 100)
  p[3][4] =  1.0;
  p[3][3] = -105.2345;
  p[3][2] =  523.388;
  p[3][1] =  12.3725;
  p[3][0] = -617.25;

  // p[4][] = (x + 1)*(x - 1.2345)*(x - 2.3456)*(x - 5)*(x - 100)
  p[4][5] =  1.0;
  p[4][4] = -107.5801;
  p[4][3] =  770.2260432;
  p[4][2] = -1215.2863928;
  p[4][1] = -646.270936;
  p[4][0] =  1447.8216;

  sandglass_t sandglass;
  if (sandglass_init_monotonic(&sandglass, SANDGLASS_CPUTIME) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < NPOLY; ++i) {
    sandglass_bench_fine(&sandglass, dmnsn_polynomial_solve(p[i], i + 1, x));
    printf("dmnsn_polynomial_solve(x^%zu): %ld\n", i + 1, sandglass.grains);
  }

  return EXIT_SUCCESS;
}
