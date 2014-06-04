/*************************************************************************
 * Copyright (C) 2014 Tavian Barnes <tavianator@tavianator.com>          *
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
  sandglass_t sandglass;
  if (sandglass_init_monotonic(&sandglass, SANDGLASS_CPUTIME) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  dmnsn_pool *pool = dmnsn_new_pool();

  dmnsn_vector vertices[] = {
    dmnsn_new_vector(1.0, 0.0, 0.0),
    dmnsn_new_vector(2.0, 2.0, 1.0),
    dmnsn_new_vector(3.0, 0.0, 2.0),
  };
  dmnsn_object *triangle = dmnsn_new_triangle(pool, vertices);
  triangle->texture = dmnsn_new_texture(pool);
  triangle->texture->pigment = dmnsn_new_pigment(pool);
  dmnsn_object_initialize(triangle);

  dmnsn_intersection intersection;
  dmnsn_line line;
  bool intersected;

  /* Intersecting case */
  line = dmnsn_new_line(dmnsn_new_vector(2.0, 1.0, -1.0), dmnsn_z);
  sandglass_bench_fine(&sandglass, {
    intersected = dmnsn_object_intersection(triangle, line, &intersection);
  });
  dmnsn_assert(intersected, "Didn't intersect");
  printf("dmnsn_triangle_intersection(true): %ld\n", sandglass.grains);

  /* Non-intersecting case */
  line = dmnsn_new_line(dmnsn_new_vector(3.0, 3.0, -1.0), dmnsn_z);
  sandglass_bench_fine(&sandglass, {
    intersected = dmnsn_object_intersection(triangle, line, &intersection);
  });
  dmnsn_assert(!intersected, "Intersected");
  printf("dmnsn_triangle_intersection(false): %ld\n", sandglass.grains);

  dmnsn_delete_pool(pool);

  return EXIT_SUCCESS;
}
