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

dmnsn_intersection *
dmnsn_fake_intersection_fn(const dmnsn_object *object, dmnsn_line line)
{
  dmnsn_intersection *intersection = dmnsn_new_intersection();
  intersection->t       = (object->bounding_box.min.z - line.x0.z)/line.n.z;
  intersection->texture = object->texture;
  return intersection;
}

void
dmnsn_randomize_bounding_box(dmnsn_object *object)
{
  double rand1, rand2;

  rand1 = 2.0*((double)rand())/RAND_MAX - 1.0;
  rand2 = 2.0*((double)rand())/RAND_MAX - 1.0;
  if (rand1 < rand2) {
    object->bounding_box.min.x = rand1;
    object->bounding_box.max.x = rand2;
  } else {
    object->bounding_box.max.x = rand1;
    object->bounding_box.min.x = rand2;
  }

  rand1 = 2.0*((double)rand())/RAND_MAX - 1.0;
  rand2 = 2.0*((double)rand())/RAND_MAX - 1.0;
  if (rand1 < rand2) {
    object->bounding_box.min.y = rand1;
    object->bounding_box.max.y = rand2;
  } else {
    object->bounding_box.max.y = rand1;
    object->bounding_box.min.y = rand2;
  }

  rand1 = 2.0*((double)rand())/RAND_MAX - 1.0;
  rand2 = 2.0*((double)rand())/RAND_MAX - 1.0;
  if (rand1 < rand2) {
    object->bounding_box.min.z = rand1;
    object->bounding_box.max.z = rand2;
  } else {
    object->bounding_box.max.z = rand1;
    object->bounding_box.min.z = rand2;
  }
}

dmnsn_bvst_node *
dmnsn_bvst_deepest_recursive(dmnsn_bvst_node *node,
                                 unsigned int depth, unsigned int *deepest)
{
  dmnsn_bvst_node *left = NULL, *right = NULL;

  if (node->contains) {
    left = dmnsn_bvst_deepest_recursive(node->contains, depth + 1, deepest);
  }
  if (node->container) {
    right = dmnsn_bvst_deepest_recursive(node->container,
                                             depth + 1, deepest);
  }

  if (right) {
    return right;
  } else if (left) {
    return left;
  } else if (depth >= *deepest) {
    *deepest = depth;
    return node;
  } else {
    return NULL;
  }
}

dmnsn_bvst_node *
dmnsn_bvst_deepest(dmnsn_bvst *tree)
{
  unsigned int deepest = 0;
  return dmnsn_bvst_deepest_recursive(tree->root, 0, &deepest);
}

int
main()
{
  dmnsn_bvst *tree;
  dmnsn_bvst_node *node;
  dmnsn_intersection *intersection;
  dmnsn_line ray;
  const unsigned int nobjects = 128;
  dmnsn_object *objects[nobjects];
  unsigned int i;
  long grains;

  sandglass_t sandglass;

  if (sandglass_init_monotonic(&sandglass, SANDGLASS_CPUTIME) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  tree = dmnsn_new_bvst();

  for (i = 0; i < nobjects; ++i) {
    objects[i] = dmnsn_new_object();

    /* Generate a bounding box in  (-1, -1, -1), (1, 1, 1) */
    dmnsn_randomize_bounding_box(objects[i]);
    objects[i]->intersection_fn = &dmnsn_fake_intersection_fn;
  }

  /* dmnsn_bvst_insert() */

  grains = 0;
  for (i = 0; i < nobjects; ++i) {
    sandglass_bench_noprecache(&sandglass,
                               dmnsn_bvst_insert(tree, objects[i]));
    sandglass.grains += grains;
    grains = sandglass.grains;
  }
  printf("dmnsn_bvst_insert(): %ld\n", sandglass.grains/nobjects);

  /* dmnsn_bvst_search() */

  ray.x0 = dmnsn_new_vector(0.0, 0.0, -2.0);
  ray.n  = dmnsn_new_vector(0.0, 0.0, 1.0);

  dmnsn_delete_intersection((*objects[0]->intersection_fn)(objects[0], ray));
  sandglass_bench_noprecache(&sandglass, {
    intersection = dmnsn_bvst_search(tree, ray);
  });
  dmnsn_delete_intersection(intersection);
  printf("dmnsn_bvst_search(): %ld\n", sandglass.grains);

  /* dmnsn_bvst_splay() */
  grains = 0;
  for (i = 0; i < nobjects; ++i) {
    node = dmnsn_bvst_deepest(tree);
    sandglass_bench_noprecache(&sandglass, dmnsn_bvst_splay(tree, node));
    sandglass.grains += grains;
    grains = sandglass.grains;
  }
  printf("dmnsn_bvst_splay(): %ld\n", sandglass.grains/nobjects);

  /* Cleanup */
  dmnsn_delete_bvst(tree);
  for (i = 0; i < nobjects; ++i) {
    dmnsn_delete_object(objects[i]);
  }

  return EXIT_SUCCESS;
}
