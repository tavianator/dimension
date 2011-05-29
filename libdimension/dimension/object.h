/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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

/** A type to represent a ray-object intersection. */
typedef struct dmnsn_intersection {
  dmnsn_line ray; /**< The ray that intersected. */
  double t;       /**< The line index that intersected. */

  /** The surface normal at the intersection point. */
  dmnsn_vector normal;

  /** The texture at the intersection point. */
  const dmnsn_texture *texture;
  /** The interior at the intersection point. */
  const dmnsn_interior *interior;
} dmnsn_intersection;

/* Forward-declare dmnsn_object */
typedef struct dmnsn_object dmnsn_object;

/**
 * Object initialization callback.
 * @param[in,out] object  The object to initialize.
 */
typedef void dmnsn_object_initialize_fn(dmnsn_object *object);

/**
 * Ray-object intersection callback.
 * @param[in]  object        The object to test.
 * @param[in]  line          The line to test.
 * @param[out] intersection  Where to store the intersection details of the
 *                           closest (if any) intersection.
 * @return Whether \p line intersected \p object.
 */
typedef bool dmnsn_object_intersection_fn(const dmnsn_object *object,
                                          dmnsn_line line,
                                          dmnsn_intersection *intersection);

/**
 * Object inside callback.
 * @param[in] object  The object to test.
 * @param[in] point   The point to test.
 * @return Whether \p point is inside \p object.
 */
typedef bool dmnsn_object_inside_fn(const dmnsn_object *object,
                                    dmnsn_vector point);

/** An object. */
struct dmnsn_object {
  dmnsn_texture *texture;   /**< Surface properties. */
  dmnsn_interior *interior; /**< Interior properties. */

  dmnsn_matrix trans;     /**< Transformation matrix. */
  dmnsn_matrix trans_inv; /**< Inverse of the transformation matrix. */

  dmnsn_bounding_box bounding_box; /**< Object bounding box. */

  /** Child objects.  This array lists objects that can be split into
      sub-objects for bounding purposes (for unions and meshes, for example). */
  dmnsn_array *children;

  dmnsn_object_initialize_fn   *initialize_fn; /**< Initialization callback. */
  dmnsn_object_intersection_fn *intersection_fn; /**< Intersection callback. */
  dmnsn_object_inside_fn       *inside_fn; /**< Inside callback. */
  dmnsn_free_fn                *free_fn; /**< Destruction callback. */

  /** Generic pointer for object info. */
  void *ptr;

  /** @internal Reference count. */
  dmnsn_refcount refcount;
};

/**
 * Allocate a dummy object.
 * @return The allocated object.
 */
dmnsn_object *dmnsn_new_object(void);

/**
 * Free an object
 * @param[in,out] object  The object to destroy.
 */
void dmnsn_delete_object(dmnsn_object *object);

/**
 * Initialize an object and potentially its children.
 * @param[in,out] object  The object to initialize.
 */
void dmnsn_initialize_object(dmnsn_object *object);


/**
 * Transform a surface normal vector.
 * @param[in] trans   The transformation matrix.
 * @param[in] normal  The normal vector to transform
 * @return The transformed normal vector.
 */
DMNSN_INLINE dmnsn_vector
dmnsn_transform_normal(dmnsn_matrix trans, dmnsn_vector normal)
{
  return dmnsn_vector_normalized(
    dmnsn_vector_sub(
      dmnsn_transform_vector(trans, normal),
      /* Optimized form of dmnsn_transform_vector(trans, dmnsn_zero) */
      dmnsn_new_vector(trans.n[0][3], trans.n[1][3], trans.n[2][3])
    )
  );
}

/**
 * Appropriately transform a ray, then test for an intersection.
 * @param[in]  object        The object to test.
 * @param[in]  line          The ray to test.
 * @param[out] intersection  Where to store the intersection details.
 * @return Whether there was an intersection.
 */
DMNSN_INLINE bool
dmnsn_object_intersection(const dmnsn_object *object, dmnsn_line line,
                          dmnsn_intersection *intersection)
{
  dmnsn_line line_trans = dmnsn_transform_line(object->trans_inv, line);
  if (object->intersection_fn(object, line_trans, intersection)) {
    /* Get us back into world coordinates */
    intersection->ray    = line;
    intersection->normal = dmnsn_transform_normal(object->trans,
                                                  intersection->normal);
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
  point = dmnsn_transform_vector(object->trans_inv, point);
  return object->inside_fn(object, point);
}
