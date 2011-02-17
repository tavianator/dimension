/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@gmail.com>          *
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

#include "dimension-impl.h"
#include <sandglass.h>
#include <stdlib.h>

static bool
dmnsn_fake_intersection_fn(const dmnsn_object *object, dmnsn_line line,
                           dmnsn_intersection *intersection)
{
  intersection->t = (object->bounding_box.min.z - line.x0.z)/line.n.z;
  intersection->normal = dmnsn_x;
  return true;
}

static bool
dmnsn_fake_inside_fn(const dmnsn_object *object, dmnsn_vector point)
{
  return true;
}

static void
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

static dmnsn_object *
dmnsn_new_fake_object(void)
{
  dmnsn_object *object = dmnsn_new_object();
  /* Generate a bounding box in  (-1, -1, -1), (1, 1, 1) */
  dmnsn_randomize_bounding_box(object);
  object->intersection_fn = &dmnsn_fake_intersection_fn;
  object->inside_fn       = &dmnsn_fake_inside_fn;
  return object;
}

int
main(void)
{
  const size_t nobjects = 10000;

  sandglass_t sandglass;
  if (sandglass_init_monotonic(&sandglass, SANDGLASS_CPUTIME) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  dmnsn_array *objects = dmnsn_new_array(sizeof(dmnsn_object *));
  for (size_t i = 0; i < nobjects; ++i) {
    dmnsn_object *object = dmnsn_new_fake_object();
    dmnsn_initialize_object(object);
    dmnsn_array_push(objects, &object);
  }

  dmnsn_prtree *tree;
  sandglass_bench_noprecache(&sandglass, {
    tree = dmnsn_new_prtree(objects);
  });
  printf("dmnsn_new_prtree(): %ld\n", sandglass.grains);

  /* dmnsn_prtree_intersection() */
  dmnsn_line ray = dmnsn_new_line(
    dmnsn_new_vector( 1.0,  1.0, -2.0),
    dmnsn_new_vector(-0.5, -0.5,  1.0)
  );
  dmnsn_intersection intersection;

  sandglass_bench_fine(&sandglass, {
    dmnsn_prtree_intersection(tree, ray, &intersection);
  });
  printf("dmnsn_prtree_intersection(): %ld\n", sandglass.grains);

  /* dmnsn_prtree_inside() */
  sandglass_bench_fine(&sandglass, {
    dmnsn_prtree_inside(tree, dmnsn_zero);
  });
  printf("dmnsn_prtree_inside(): %ld\n", sandglass.grains);

  /* Cleanup */
  dmnsn_delete_prtree(tree);
  for (size_t i = 0; i < nobjects; ++i) {
    dmnsn_object *object;
    dmnsn_array_get(objects, i, &object);
    dmnsn_delete_object(object);
  }

  dmnsn_delete_array(objects);
  return EXIT_SUCCESS;
}
