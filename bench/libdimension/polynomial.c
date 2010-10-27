/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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

#include "dimension.h"
#include <sandglass.h>
#include <stdlib.h>

int
main(void)
{
  double poly[6], x[5];
  // poly[] = (x + 1)*(x - 1.2345)*(x - 2.3456)*(x - 5)*(x - 100)
  poly[5] =  1.0;
  poly[4] = -107.5801;
  poly[3] =  770.2260432;
  poly[2] = -1215.2863928;
  poly[1] = -646.270936;
  poly[0] =  1447.8216;

  sandglass_t sandglass;
  if (sandglass_init_monotonic(&sandglass, SANDGLASS_CPUTIME) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  sandglass_bench_fine(&sandglass, dmnsn_solve_polynomial(poly, 5, x));
  printf("dmnsn_solve_polynomial(): %ld\n", sandglass.grains);

  return EXIT_SUCCESS;
}
