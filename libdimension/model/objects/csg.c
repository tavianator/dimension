/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Library.                           *
 *                                                                       *
 * The Dimension Library is free software; you can redistribute it and/  *
 * or modify it under the terms of the GNU Lesser General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Library is distributed in the hope that it will be      *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

/**
 * @file
 * Constructive solid geometry.
 */

#include "internal.h"
#include "internal/bvh.h"
#include "dimension/model.h"
#include <stdlib.h>

////////////
// Unions //
////////////

typedef struct {
  dmnsn_object object;
  dmnsn_bvh *bvh;
} dmnsn_csg_union;

/// CSG union intersection callback.
static bool
dmnsn_csg_union_intersection_fn(const dmnsn_object *object,
                                dmnsn_ray ray,
                                dmnsn_intersection *intersection)
{
  const dmnsn_csg_union *csg = (const dmnsn_csg_union *)object;
  return dmnsn_bvh_intersection(csg->bvh, ray, intersection, true);
}

/// CSG union inside callback.
static bool
dmnsn_csg_union_inside_fn(const dmnsn_object *object, dmnsn_vector point)
{
  const dmnsn_csg_union *csg = (const dmnsn_csg_union *)object;
  return dmnsn_bvh_inside(csg->bvh, point);
}

/// CSG union precomputation callback.
static void
dmnsn_csg_union_precompute_fn(dmnsn_object *object)
{
  dmnsn_csg_union *csg = (dmnsn_csg_union *)object;
  csg->object.trans_inv = dmnsn_identity_matrix();

  dmnsn_bvh *bvh = dmnsn_new_bvh(csg->object.children, DMNSN_BVH_PRTREE);
  csg->bvh = bvh;
  csg->object.aabb = dmnsn_bvh_aabb(bvh);
}

/// CSG union vtable.
static const dmnsn_object_vtable dmnsn_csg_union_vtable = {
  .intersection_fn = dmnsn_csg_union_intersection_fn,
  .inside_fn = dmnsn_csg_union_inside_fn,
  .precompute_fn = dmnsn_csg_union_precompute_fn,
};

/// CSG union destruction callback.
static void
dmnsn_csg_union_cleanup(void *ptr)
{
  dmnsn_csg_union *csg = ptr;
  dmnsn_delete_bvh(csg->bvh);
}

// Bulk-load a union
dmnsn_object *
dmnsn_new_csg_union(dmnsn_pool *pool, dmnsn_array *objects)
{
  dmnsn_csg_union *csg = DMNSN_PALLOC_TIDY(pool, dmnsn_csg_union, dmnsn_csg_union_cleanup);
  csg->bvh = NULL;

  dmnsn_object *object = &csg->object;
  dmnsn_init_object(object);

  object->vtable = &dmnsn_csg_union_vtable;
  object->children = objects;
  object->split_children = true;

  return object;
}

/**
 * Generic CSG intersection callback.
 * @param[in]  csg           The CSG object.
 * @param[in]  ray           The intersection ray.
 * @param[out] intersection  The intersection data.
 * @param[in]  inside1       Whether the first object is allowed inside the
 *                           second object.
 * @param[in]  inside2       Whether the second object is allowed inside the
 *                           first object.
 * @return Whether \p ray intersected \p csg.
 */
static bool
dmnsn_csg_intersection_fn(const dmnsn_object *csg, dmnsn_ray ray,
                          dmnsn_intersection *intersection,
                          bool inside1, bool inside2)
{
  const dmnsn_object *A = *(dmnsn_object **)dmnsn_array_first(csg->children);
  const dmnsn_object *B = *(dmnsn_object **)dmnsn_array_last(csg->children);

  dmnsn_intersection i1, i2;
  bool is_i1 = dmnsn_object_intersection(A, ray, &i1);
  bool is_i2 = dmnsn_object_intersection(B, ray, &i2);

  double oldt = 0.0;
  while (is_i1) {
    i1.ray = ray;
    i1.t += oldt;
    oldt = i1.t + dmnsn_epsilon;

    dmnsn_vector point = dmnsn_ray_point(i1.ray, i1.t);
    if (inside2 ^ dmnsn_object_inside(B, point)) {
      dmnsn_ray newray = ray;
      newray.x0 = dmnsn_ray_point(ray, i1.t);
      newray    = dmnsn_ray_add_epsilon(newray);
      is_i1 = dmnsn_object_intersection(A, newray, &i1);
    } else {
      break;
    }
  }

  oldt = 0.0;
  while (is_i2) {
    i2.ray = ray;
    i2.t += oldt;
    oldt = i2.t + dmnsn_epsilon;

    dmnsn_vector point = dmnsn_ray_point(i2.ray, i2.t);
    if (inside1 ^ dmnsn_object_inside(A, point)) {
      dmnsn_ray newray = ray;
      newray.x0 = dmnsn_ray_point(ray, i2.t);
      newray    = dmnsn_ray_add_epsilon(newray);
      is_i2 = dmnsn_object_intersection(B, newray, &i2);
    } else {
      break;
    }
  }

  if (is_i1 && is_i2) {
    if (i1.t < i2.t) {
      *intersection = i1;
    } else {
      *intersection = i2;
    }
  } else if (is_i1) {
    *intersection = i1;
  } else if (is_i2) {
    *intersection = i2;
  } else {
    return false;
  }

  return true;
}

///////////////////
// Intersections //
///////////////////

/// CSG intersection intersection callback.
static bool
dmnsn_csg_intersection_intersection_fn(const dmnsn_object *csg,
                                       dmnsn_ray ray,
                                       dmnsn_intersection *intersection)
{
  return dmnsn_csg_intersection_fn(csg, ray, intersection, true, true);
}

/// CSG intersection inside callback.
static bool
dmnsn_csg_intersection_inside_fn(const dmnsn_object *csg, dmnsn_vector point)
{
  const dmnsn_object *A = *(dmnsn_object **)dmnsn_array_first(csg->children);
  const dmnsn_object *B = *(dmnsn_object **)dmnsn_array_last(csg->children);
  return dmnsn_object_inside(A, point) && dmnsn_object_inside(B, point);
}

/// CSG intersection precomputation callback.
static void
dmnsn_csg_intersection_precompute_fn(dmnsn_object *csg)
{
  dmnsn_object *A = *(dmnsn_object **)dmnsn_array_first(csg->children);
  dmnsn_object *B = *(dmnsn_object **)dmnsn_array_last(csg->children);

  csg->trans_inv = dmnsn_identity_matrix();
  csg->aabb.min = dmnsn_vector_max(A->aabb.min, B->aabb.min);
  csg->aabb.max = dmnsn_vector_min(A->aabb.max, B->aabb.max);
}

/// CSG intersection vtable.
static const dmnsn_object_vtable dmnsn_csg_intersection_vtable = {
  .intersection_fn = dmnsn_csg_intersection_intersection_fn,
  .inside_fn = dmnsn_csg_intersection_inside_fn,
  .precompute_fn = dmnsn_csg_intersection_precompute_fn,
};

dmnsn_object *
dmnsn_new_csg_intersection(dmnsn_pool *pool, dmnsn_object *A, dmnsn_object *B)
{
  dmnsn_object *csg = dmnsn_new_object(pool);
  csg->vtable = &dmnsn_csg_intersection_vtable;

  csg->children = DMNSN_PALLOC_ARRAY(pool, dmnsn_object *);
  dmnsn_array_push(csg->children, &A);
  dmnsn_array_push(csg->children, &B);

  return csg;
}

/////////////////
// Differences //
/////////////////

/// CSG difference intersection callback.
static bool
dmnsn_csg_difference_intersection_fn(const dmnsn_object *csg,
                                     dmnsn_ray ray,
                                     dmnsn_intersection *intersection)
{
  return dmnsn_csg_intersection_fn(csg, ray, intersection, true, false);
}

/// CSG difference inside callback.
static bool
dmnsn_csg_difference_inside_fn(const dmnsn_object *csg, dmnsn_vector point)
{
  const dmnsn_object *A = *(dmnsn_object **)dmnsn_array_first(csg->children);
  const dmnsn_object *B = *(dmnsn_object **)dmnsn_array_last(csg->children);
  return dmnsn_object_inside(A, point)  && !dmnsn_object_inside(B, point);
}

/// CSG difference precomputation callback.
static void
dmnsn_csg_difference_precompute_fn(dmnsn_object *csg)
{
  dmnsn_object *A = *(dmnsn_object **)dmnsn_array_first(csg->children);

  csg->trans_inv = dmnsn_identity_matrix();
  csg->aabb = A->aabb;
}

/// CSG difference vtable.
static const dmnsn_object_vtable dmnsn_csg_difference_vtable = {
  .intersection_fn = dmnsn_csg_difference_intersection_fn,
  .inside_fn = dmnsn_csg_difference_inside_fn,
  .precompute_fn = dmnsn_csg_difference_precompute_fn,
};

dmnsn_object *
dmnsn_new_csg_difference(dmnsn_pool *pool, dmnsn_object *A, dmnsn_object *B)
{
  dmnsn_object *csg = dmnsn_new_object(pool);
  csg->vtable = &dmnsn_csg_difference_vtable;

  csg->children = DMNSN_PALLOC_ARRAY(pool, dmnsn_object *);
  dmnsn_array_push(csg->children, &A);
  dmnsn_array_push(csg->children, &B);

  return csg;
}

////////////
// Merges //
////////////

/// CSG merge intersection callback.
static bool
dmnsn_csg_merge_intersection_fn(const dmnsn_object *csg,
                                dmnsn_ray ray,
                                dmnsn_intersection *intersection)
{
  return dmnsn_csg_intersection_fn(csg, ray, intersection, false, false);
}

/// CSG merge inside callback.
static bool
dmnsn_csg_merge_inside_fn(const dmnsn_object *csg, dmnsn_vector point)
{
  const dmnsn_object *A = *(dmnsn_object **)dmnsn_array_first(csg->children);
  const dmnsn_object *B = *(dmnsn_object **)dmnsn_array_last(csg->children);
  return dmnsn_object_inside(A, point) || dmnsn_object_inside(B, point);
}

/// CSG merge precomputation callback.
static void
dmnsn_csg_merge_precompute_fn(dmnsn_object *csg)
{
  dmnsn_object *A = *(dmnsn_object **)dmnsn_array_first(csg->children);
  dmnsn_object *B = *(dmnsn_object **)dmnsn_array_last(csg->children);

  csg->trans_inv = dmnsn_identity_matrix();
  csg->aabb.min = dmnsn_vector_min(A->aabb.min, B->aabb.min);
  csg->aabb.max = dmnsn_vector_max(A->aabb.max, B->aabb.max);
}

/// CSG merge vtable.
static const dmnsn_object_vtable dmnsn_csg_merge_vtable = {
  .intersection_fn = dmnsn_csg_merge_intersection_fn,
  .inside_fn = dmnsn_csg_merge_inside_fn,
  .precompute_fn = dmnsn_csg_merge_precompute_fn,
};

dmnsn_object *
dmnsn_new_csg_merge(dmnsn_pool *pool, dmnsn_object *A, dmnsn_object *B)
{
  dmnsn_object *csg = dmnsn_new_object(pool);
  csg->vtable = &dmnsn_csg_merge_vtable;

  csg->children = DMNSN_PALLOC_ARRAY(pool, dmnsn_object *);
  dmnsn_array_push(csg->children, &A);
  dmnsn_array_push(csg->children, &B);

  return csg;
}
