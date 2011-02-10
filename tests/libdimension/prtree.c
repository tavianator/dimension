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

/*
 * Basic tests of PR-trees
 */

#include "dimension-impl.h"
#include "../../libdimension/prtree.c" /* For DMNSN_PRTREE_B */
#include <stdio.h>
#include <stdlib.h>

unsigned int calls = 0;

static bool
dmnsn_fake_intersection_fn(const dmnsn_object *object, dmnsn_line line,
                           dmnsn_intersection *intersection)
{
  intersection->t = (object->bounding_box.min.z - line.x0.z)/line.n.z;
  intersection->normal = dmnsn_x;
  ++calls;
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

int
main(void)
{
  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  const size_t nobjects = 128;
  dmnsn_scene *scene = dmnsn_new_scene();

  for (size_t i = 0; i < nobjects; ++i) {
    dmnsn_object *object = dmnsn_new_object();
    dmnsn_randomize_bounding_box(object);
    object->intersection_fn = &dmnsn_fake_intersection_fn;
    dmnsn_initialize_object(object);
    dmnsn_array_push(scene->objects, &object);
  }

  dmnsn_prtree *prtree = dmnsn_new_prtree(scene->objects);

  dmnsn_intersection intersection;
  dmnsn_line ray = dmnsn_new_line(
    dmnsn_new_vector(0.0, 0.0, -2.0),
    dmnsn_new_vector(0.0, 0.0, 1.0)
  );

  if (!dmnsn_prtree_intersection(prtree, ray, &intersection)) {
    fprintf(stderr, "--- Didn't find intersection! ---\n");
    return EXIT_FAILURE;
  }

  if (calls > DMNSN_PRTREE_B) {
    fprintf(stderr,
            "--- Too many intersection function calls: %u! ---\n",
            calls);
    return EXIT_FAILURE;
  }

  dmnsn_delete_prtree(prtree);
  dmnsn_delete_scene(scene);
  return EXIT_SUCCESS;
}
