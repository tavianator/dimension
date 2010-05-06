/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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

#include "../../libdimension/dimension_impl.h"
#include <sandglass.h>
#include <stdlib.h>

bool
dmnsn_fake_intersection_fn(const dmnsn_object *object, dmnsn_line line,
                           dmnsn_intersection *intersection)
{
  intersection->t = (object->bounding_box.min.z - line.x0.z)/line.n.z;
  return true;
}

void
dmnsn_randomize_bounding_box(dmnsn_object *object)
{
  dmnsn_vector a, b;

  a.x = 2.0*((double)rand())/RAND_MAX - 1.0;
  a.y = 2.0*((double)rand())/RAND_MAX - 1.0;
  a.z = 2.0*((double)rand())/RAND_MAX - 1.0;

  b.x = 2.0*((double)rand())/RAND_MAX - 1.0;
  b.y = 2.0*((double)rand())/RAND_MAX - 1.0;
  b.z = 2.0*((double)rand())/RAND_MAX - 1.0;

  object->bounding_box.min = dmnsn_vector_min(a, b);
  object->bounding_box.max = dmnsn_vector_max(a, b);
}

int
main()
{
  dmnsn_prtree *tree;
  dmnsn_intersection intersection;
  dmnsn_line ray;
  const unsigned int nobjects = 128;
  dmnsn_array *objects;
  unsigned int i;

  sandglass_t sandglass;
  if (sandglass_init_monotonic(&sandglass, SANDGLASS_CPUTIME) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  objects = dmnsn_new_array(sizeof(dmnsn_object *));
  for (i = 0; i < nobjects; ++i) {
    dmnsn_object *object = dmnsn_new_object();
    /* Generate a bounding box in  (-1, -1, -1), (1, 1, 1) */
    dmnsn_randomize_bounding_box(object);
    object->intersection_fn = &dmnsn_fake_intersection_fn;
    dmnsn_array_push(objects, &object);
  }

  sandglass_bench_noprecache(&sandglass, {
    tree = dmnsn_new_prtree(objects);
  });
  printf("dmnsn_new_prtree(): %ld\n", sandglass.grains);

  /* dmnsn_prtree_search() */
  ray.x0 = dmnsn_new_vector(0.0, 0.0, -2.0);
  ray.n  = dmnsn_new_vector(0.0, 0.0, 1.0);

  sandglass_bench_fine(&sandglass, {
    dmnsn_prtree_search(tree, ray, &intersection);
  });
  printf("dmnsn_prtree_search(): %ld\n", sandglass.grains);

  /* Cleanup */
  dmnsn_delete_prtree(tree);
  for (i = 0; i < nobjects; ++i) {
    dmnsn_object *object;
    dmnsn_array_get(objects, i, &object);
    dmnsn_delete_object(object);
  }

  dmnsn_delete_array(objects);
  return EXIT_SUCCESS;
}
