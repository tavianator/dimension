/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "../platform.c"
#include "../threads.c"
#include "../future.c"
#include "../bvh.c"
#include "../prtree.c"
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

static dmnsn_bounding_box
dmnsn_fake_bounding_fn(const dmnsn_object *object, dmnsn_matrix trans)
{
  dmnsn_vector a, b;

  a.x = 2.0*((double)rand())/RAND_MAX - 1.0;
  a.y = 2.0*((double)rand())/RAND_MAX - 1.0;
  a.z = 2.0*((double)rand())/RAND_MAX - 1.0;

  b.x = 2.0*((double)rand())/RAND_MAX - 1.0;
  b.y = 2.0*((double)rand())/RAND_MAX - 1.0;
  b.z = 2.0*((double)rand())/RAND_MAX - 1.0;

  return dmnsn_new_bounding_box(dmnsn_vector_min(a, b), dmnsn_vector_max(a, b));
}

static dmnsn_object_vtable dmnsn_fake_vtable = {
  .intersection_fn = dmnsn_fake_intersection_fn,
  .inside_fn = dmnsn_fake_inside_fn,
  .bounding_fn = dmnsn_fake_bounding_fn,
};

static dmnsn_object *
dmnsn_new_fake_object(dmnsn_pool *pool)
{
  dmnsn_object *object = dmnsn_new_object(pool);
  object->vtable = &dmnsn_fake_vtable;
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

  dmnsn_pool *pool = dmnsn_new_pool();
  dmnsn_array *objects = DMNSN_PALLOC_ARRAY(pool, dmnsn_object *);
  dmnsn_texture *texture = dmnsn_new_texture(pool);
  texture->pigment = dmnsn_new_pigment(pool);
  for (size_t i = 0; i < nobjects; ++i) {
    dmnsn_object *object = dmnsn_new_fake_object(pool);
    object->texture = texture;
    dmnsn_object_precompute(object);
    dmnsn_array_push(objects, &object);
  }

  dmnsn_bvh *bvh;
  sandglass_bench_noprecache(&sandglass, {
    bvh = dmnsn_new_bvh(objects, DMNSN_BVH_PRTREE);
  });
  printf("dmnsn_new_bvh(DMNSN_BVH_PRTREE): %ld\n", sandglass.grains);

  // dmnsn_bvh_intersection()
  dmnsn_line ray = dmnsn_new_line(
    dmnsn_new_vector( 1.0,  1.0, -2.0),
    dmnsn_new_vector(-0.5, -0.5,  1.0)
  );
  dmnsn_intersection intersection;

  sandglass_bench_fine(&sandglass, {
    dmnsn_bvh_intersection(bvh, ray, &intersection, true);
  });
  printf("dmnsn_bvh_intersection(): %ld\n", sandglass.grains);

  sandglass_bench_fine(&sandglass, {
    dmnsn_bvh_intersection(bvh, ray, &intersection, false);
  });
  printf("dmnsn_bvh_intersection(nocache): %ld\n", sandglass.grains);

  // dmnsn_bvh_inside()
  sandglass_bench_fine(&sandglass, {
    dmnsn_bvh_inside(bvh, dmnsn_zero);
  });
  printf("dmnsn_bvh_inside(): %ld\n", sandglass.grains);

  // Cleanup
  dmnsn_delete_bvh(bvh);
  dmnsn_delete_pool(pool);
  return EXIT_SUCCESS;
}
