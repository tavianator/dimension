/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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

dmnsn_intersection *
dmnsn_fake_intersection_fn(const dmnsn_object *object, dmnsn_line line)
{
  dmnsn_intersection *intersection = dmnsn_new_intersection();
  intersection->t       = (object->min.z - line.x0.z)/line.n.z;
  intersection->texture = object->texture;
  return intersection;
}

dmnsn_vector
dmnsn_random_vector(dmnsn_vector min)
{
  dmnsn_vector ret;
  ret.x = 2.0*((double)rand())/RAND_MAX - 1.0;
  ret.y = 2.0*((double)rand())/RAND_MAX - 1.0;
  ret.z = 2.0*((double)rand())/RAND_MAX - 1.0;
  if (ret.x < min.x) ret.x += 2.0;
  if (ret.y < min.y) ret.y += 2.0;
  if (ret.z < min.z) ret.z += 2.0;
  return ret;
}

int
main()
{
  dmnsn_kD_splay_tree *tree;
  dmnsn_object        *object;
  dmnsn_array         *objects;
  dmnsn_intersection  *intersection;
  dmnsn_line ray;
  unsigned int i;
  long grains;
  const unsigned int nobjects = 128;

  sandglass_t sandglass;
  sandglass_attributes_t attr = { SANDGLASS_MONOTONIC, SANDGLASS_CPUTIME };

  if (sandglass_create(&sandglass, &attr, &attr) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  tree = dmnsn_new_kD_splay_tree();
  objects = dmnsn_new_array(sizeof(dmnsn_object *));

  for (i = 0; i < nobjects; ++i) {
    object = dmnsn_new_object();
    if (!object) {
      fprintf(stderr, "--- Couldn't allocate object! ---\n");
      return EXIT_FAILURE;
    }

    /* Generate a bounding box in  (-1, -1, -1), (1, 1, 1) */
    object->min = dmnsn_random_vector(dmnsn_vector_construct(-1.0, -1.0, -1.0));
    object->max = dmnsn_random_vector(object->min);
    object->intersection_fn = &dmnsn_fake_intersection_fn;
    dmnsn_array_push(objects, &object);
  }

  /* dmnsn_kD_splay_insert() */

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);
  sandglass.baseline = sandglass.grains;

  grains = 0;
  for (i = 0; i < nobjects; ++i) {
    dmnsn_array_get(objects, i, &object);

    sandglass_begin(&sandglass);
      dmnsn_kD_splay_insert(tree, object);
    sandglass_elapse(&sandglass);

    sandglass.grains += grains;
    grains = sandglass.grains;
  }
  printf("dmnsn_kD_splay_insert(): %ld\n", sandglass.grains/nobjects);

  /* dmnsn_kD_splay_search() */

  ray.x0 = dmnsn_vector_construct(0.0, 0.0, -2.0);
  ray.n  = dmnsn_vector_construct(0.0, 0.0, 1.0);

  dmnsn_delete_intersection((*object->intersection_fn)(object, ray));
  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);
  sandglass.baseline = sandglass.grains;

  sandglass_begin(&sandglass);
    intersection = dmnsn_kD_splay_search(tree, ray);
  sandglass_elapse(&sandglass);
  dmnsn_delete_intersection(intersection);
  printf("dmnsn_kD_splay_search(): %ld\n", sandglass.grains);

  /* Cleanup */
  dmnsn_delete_kD_splay_tree(tree);
  for (i = 0; i < nobjects; ++i) {
    dmnsn_array_get(objects, i, &object);
    dmnsn_delete_object(object);
  }
  dmnsn_delete_array(objects);

  return EXIT_SUCCESS;
}
