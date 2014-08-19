/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Objects.
 */

#include <stdbool.h>

/* Forward-declare dmnsn_object */
typedef struct dmnsn_object dmnsn_object;

/** A type to represent a ray-object intersection. */
typedef struct dmnsn_intersection {
  dmnsn_ray ray; /**< The ray that intersected. */
  double t;      /**< The ray index that intersected. */

  /** The surface normal at the intersection point. */
  dmnsn_vector normal;

  /** The object of intersection. */
  const dmnsn_object *object;
} dmnsn_intersection;

/**
 * Ray-object intersection callback.
 * @param[in]  object        The object to test.
 * @param[in]  ray           The ray to test.
 * @param[out] intersection  Where to store the intersection details of the
 *                           closest (if any) intersection.
 * @return Whether \p ray intersected \p object.
 */
typedef bool dmnsn_object_intersection_fn(const dmnsn_object *object, dmnsn_ray ray, dmnsn_intersection *intersection);

/**
 * Object inside callback.
 * @param[in] object  The object to test.
 * @param[in] point   The point to test.
 * @return Whether \p point is inside \p object.
 */
typedef bool dmnsn_object_inside_fn(const dmnsn_object *object, dmnsn_vector point);

/**
 * Object bounding callback.
 * @param[in,out] object  The object to bound.
 * @param[in,out] trans  The effective transformation for the object.
 * @return A world-coordinate bounding box computed for the object.
 */
typedef dmnsn_aabb dmnsn_object_bounding_fn(const dmnsn_object *object, dmnsn_matrix trans);

/**
 * Object precomputation callback.
 * @param[in,out] object  The object to precompute.
 */
typedef void dmnsn_object_precompute_fn(dmnsn_object *object);

/** Object callbacks. */
typedef struct dmnsn_object_vtable {
  dmnsn_object_intersection_fn *intersection_fn; /**< Intersection callback. */
  dmnsn_object_inside_fn *inside_fn; /**< Inside callback. */
  dmnsn_object_bounding_fn *bounding_fn; /**< Bounding callback. */
  dmnsn_object_precompute_fn *precompute_fn; /**< Precomputation callback. */
} dmnsn_object_vtable;

/** An object. */
struct dmnsn_object {
  const dmnsn_object_vtable *vtable; /**< Callbacks. */

  dmnsn_texture *texture;  /**< Surface properties. */
  dmnsn_interior *interior; /**< Interior properties. */

  dmnsn_matrix trans; /**< Transformation matrix. */
  dmnsn_matrix intrinsic_trans; /**< Transformations intrinsic to the object. */

  dmnsn_array *children; /**< Child objects. */
  bool split_children;   /**< Whether the child objects can be split. */

  /* Precomputed values */
  bool precomputed; /**< @internal Whether the object is precomputed yet. */
  dmnsn_matrix trans_inv; /**< Inverse of the transformation matrix. */
  dmnsn_matrix pigment_trans; /**< Inverse transformation for the texture. */
  dmnsn_aabb aabb; /**< Bounding box in world coordinates. */
};

/**
 * Allocate a dummy object.
 * @param[in] pool  The memory pool to allocate from.
 * @return The allocated object.
 */
dmnsn_object *dmnsn_new_object(dmnsn_pool *pool);

/**
 * Initialize a dmnsn_object field.
 * @param[out] object  The object to initialize.
 */
void dmnsn_init_object(dmnsn_object *object);

/**
 * Precompute values for an object and its children.
 * @param[in,out] object  The object to precompute.
 */
void dmnsn_object_precompute(dmnsn_object *object);

/**
 * Appropriately transform a ray, then test for an intersection.
 * @param[in]  object        The object to test.
 * @param[in]  ray           The ray to test.
 * @param[out] intersection  Where to store the intersection details.
 * @return Whether there was an intersection.
 */
DMNSN_INLINE bool
dmnsn_object_intersection(const dmnsn_object *object, dmnsn_ray ray,
                          dmnsn_intersection *intersection)
{
  dmnsn_ray ray_trans = dmnsn_transform_ray(object->trans_inv, ray);
  intersection->object = NULL;
  if (object->vtable->intersection_fn(object, ray_trans, intersection)) {
    /* Get us back into world coordinates */
    intersection->ray = ray;
    intersection->normal = dmnsn_vector_normalized(
      dmnsn_transform_normal(object->trans_inv, intersection->normal)
    );
    if (!intersection->object) {
      intersection->object = object;
    }

    dmnsn_assert(!dmnsn_isnan(intersection->t), "Intersection point is NaN.");
    dmnsn_assert(!dmnsn_vector_isnan(intersection->normal), "Intersection normal is NaN.");

    return true;
  } else {
    return false;
  }
}

/**
 * Appropriately transform a point, then test for containment.
 * @param[in] object  The object to test.
 * @param[in] point   The point to test.
 * @return Whether \p point was inside \p object.
 */
DMNSN_INLINE bool
dmnsn_object_inside(const dmnsn_object *object, dmnsn_vector point)
{
  point = dmnsn_transform_point(object->trans_inv, point);
  return object->vtable->inside_fn(object, point);
}
