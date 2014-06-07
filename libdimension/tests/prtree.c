/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "../platform.c"
#include "../threads.c"
#include "../future.c"
#include "../bvh.c"
#include "../prtree.c"
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

static const dmnsn_object_vtable dmnsn_fake_vtable = {
  .intersection_fn = dmnsn_fake_intersection_fn,
};

static dmnsn_object *
dmnsn_new_fake_object(dmnsn_pool *pool)
{
  dmnsn_object *object = dmnsn_new_object(pool);
  dmnsn_randomize_bounding_box(object);
  object->vtable = &dmnsn_fake_vtable;
  object->trans_inv = dmnsn_identity_matrix();
  return object;
}

int
main(void)
{
  // Treat warnings as errors for tests
  dmnsn_die_on_warnings(true);

  dmnsn_pool *pool = dmnsn_new_pool();

  const size_t nobjects = 128;
  dmnsn_array *objects = DMNSN_PALLOC_ARRAY(pool, dmnsn_object *);

  for (size_t i = 0; i < nobjects; ++i) {
    dmnsn_object *object = dmnsn_new_fake_object(pool);
    dmnsn_array_push(objects, &object);
  }

  dmnsn_bvh *bvh = dmnsn_new_bvh(objects, DMNSN_BVH_PRTREE);

  dmnsn_intersection intersection;
  dmnsn_line ray = dmnsn_new_line(
    dmnsn_new_vector(0.0, 0.0, -2.0),
    dmnsn_new_vector(0.0, 0.0, 1.0)
  );

  if (!dmnsn_bvh_intersection(bvh, ray, &intersection, true)) {
    fprintf(stderr, "--- Didn't find intersection! ---\n");
    return EXIT_FAILURE;
  }

  if (calls > DMNSN_PRTREE_B) {
    fprintf(stderr,
            "--- Too many intersection function calls: %u! ---\n",
            calls);
    return EXIT_FAILURE;
  }

  dmnsn_delete_bvh(bvh);
  dmnsn_delete_pool(pool);
  return EXIT_SUCCESS;
}
