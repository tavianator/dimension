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

void
dmnsn_randomize_bounding_box(dmnsn_object *object)
{
  double rand1, rand2;

  rand1 = 2.0*((double)rand())/RAND_MAX - 1.0;
  rand2 = 2.0*((double)rand())/RAND_MAX - 1.0;
  if (rand1 < rand2) {
    object->min.x = rand1;
    object->max.x = rand2;
  } else {
    object->max.x = rand1;
    object->min.x = rand2;
  }

  rand1 = 2.0*((double)rand())/RAND_MAX - 1.0;
  rand2 = 2.0*((double)rand())/RAND_MAX - 1.0;
  if (rand1 < rand2) {
    object->min.y = rand1;
    object->max.y = rand2;
  } else {
    object->max.y = rand1;
    object->min.y = rand2;
  }

  rand1 = 2.0*((double)rand())/RAND_MAX - 1.0;
  rand2 = 2.0*((double)rand())/RAND_MAX - 1.0;
  if (rand1 < rand2) {
    object->min.z = rand1;
    object->max.z = rand2;
  } else {
    object->max.z = rand1;
    object->min.z = rand2;
  }
}

dmnsn_kD_splay_node *
dmnsn_kD_splay_deepest_recursive(dmnsn_kD_splay_node *node,
                                 unsigned int depth, unsigned int *deepest)
{
  dmnsn_kD_splay_node *left = NULL, *right = NULL;
  unsigned int left_depth = 0, right_depth = 0;
  if (node->contains) {
    left = dmnsn_kD_splay_deepest_recursive(node->contains,
                                            depth + 1, &left_depth);
  }
  if (node->container) {
    right = dmnsn_kD_splay_deepest_recursive(node->container,
                                             depth + 1, &right_depth);
  }

  if (right && right_depth > left_depth) {
    *deepest = right_depth;
    return right;
  } else if (left) {
    *deepest = left_depth;
    return left;
  } else {
    *deepest = depth;
    return node;
  }
}

dmnsn_kD_splay_node *
dmnsn_kD_splay_deepest(dmnsn_kD_splay_tree *tree)
{
  unsigned int deepest = 0;
  return dmnsn_kD_splay_deepest_recursive(tree->root, 0, &deepest);
}

int
main()
{
  dmnsn_kD_splay_tree *tree;
  dmnsn_kD_splay_node *node;
  dmnsn_intersection *intersection;
  dmnsn_line ray;
  const unsigned int nobjects = 128;
  dmnsn_object *objects[nobjects];
  unsigned int i;
  long grains;

  sandglass_t sandglass;
  sandglass_attributes_t attr = { SANDGLASS_MONOTONIC, SANDGLASS_CPUTIME };

  if (sandglass_create(&sandglass, &attr, &attr) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  tree = dmnsn_new_kD_splay_tree();

  for (i = 0; i < nobjects; ++i) {
    objects[i] = dmnsn_new_object();
    if (!objects[i]) {
      fprintf(stderr, "--- Couldn't allocate object! ---\n");
      return EXIT_FAILURE;
    }

    /* Generate a bounding box in  (-1, -1, -1), (1, 1, 1) */
    dmnsn_randomize_bounding_box(objects[i]);
    objects[i]->intersection_fn = &dmnsn_fake_intersection_fn;
  }

  /* dmnsn_kD_splay_insert() */

  grains = 0;
  for (i = 0; i < nobjects; ++i) {
    sandglass_bench_noprecache(&sandglass,
                               dmnsn_kD_splay_insert(tree, objects[i]));
    sandglass.grains += grains;
    grains = sandglass.grains;
  }
  printf("dmnsn_kD_splay_insert(): %ld\n", sandglass.grains/nobjects);

  /* dmnsn_kD_splay_search() */

  ray.x0 = dmnsn_vector_construct(0.0, 0.0, -2.0);
  ray.n  = dmnsn_vector_construct(0.0, 0.0, 1.0);

  dmnsn_delete_intersection((*objects[0]->intersection_fn)(objects[0], ray));
  sandglass_bench_noprecache(&sandglass, {
    intersection = dmnsn_kD_splay_search(tree, ray);
  });
  dmnsn_delete_intersection(intersection);
  printf("dmnsn_kD_splay_search(): %ld\n", sandglass.grains);

  /* dmnsn_kD_splay() */
  grains = 0;
  for (i = 0; i < nobjects; ++i) {
    node = dmnsn_kD_splay_deepest(tree);
    sandglass_bench_noprecache(&sandglass, dmnsn_kD_splay(tree, node));
    sandglass.grains += grains;
    grains = sandglass.grains;
  }
  printf("dmnsn_kD_splay(): %ld\n", sandglass.grains/nobjects);

  /* Cleanup */
  dmnsn_delete_kD_splay_tree(tree);
  for (i = 0; i < nobjects; ++i) {
    dmnsn_delete_object(objects[i]);
  }

  return EXIT_SUCCESS;
}
