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
  dmnsn_line ray; /**< The ray that intersected. */
  double t;       /**< The line index that intersected. */

  /** The surface normal at the intersection point. */
  dmnsn_vector normal;

  /** The object of intersection. */
  const dmnsn_object *object;
} dmnsn_intersection;

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

/**
 * Object destruction callback.
 * @param[in,out] object  The object to delete.
 */
typedef void dmnsn_object_free_fn(dmnsn_object *object);

/** An object. */
struct dmnsn_object {
  dmnsn_texture  *texture;  /**< Surface properties. */
  dmnsn_interior *interior; /**< Interior properties. */

  dmnsn_matrix trans; /**< Transformation matrix. */
  dmnsn_matrix trans_inv; /**< Inverse of the transformation matrix. */
  dmnsn_matrix intrinsic_trans; /**< Transformations intrinsic to the object. */
  dmnsn_matrix pigment_trans; /**< Inverse transformation for the texture. */

  dmnsn_bounding_box bounding_box; /**< Object bounding box. */

  dmnsn_array *children; /**< Child objects. */
  bool split_children;   /**< Whether the child objects can be split. */

  dmnsn_object_initialize_fn   *initialize_fn; /**< Initialization callback. */
  dmnsn_object_intersection_fn *intersection_fn; /**< Intersection callback. */
  dmnsn_object_inside_fn       *inside_fn; /**< Inside callback. */
  dmnsn_object_free_fn         *free_fn; /**< Destruction callback. */

  DMNSN_REFCOUNT; /**< Reference count. */
  bool initialized; /**< @internal Whether the object is initialized yet. */
};

/**
 * Allocate a dummy object.
 * @return The allocated object.
 */
dmnsn_object *dmnsn_new_object(void);

/**
 * Initialize a dmnsn_object field.
 * @param[out] object  The object to initialize.
 */
void dmnsn_init_object(dmnsn_object *object);

/**
 * Free an object
 * @param[in,out] object  The object to destroy.
 */
void dmnsn_delete_object(dmnsn_object *object);

/**
 * Initialize an object and potentially its children.
 * @param[in,out] object  The object to initialize.
 */
void dmnsn_object_initialize(dmnsn_object *object);

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
  intersection->object = NULL;
  if (object->intersection_fn(object, line_trans, intersection)) {
    /* Get us back into world coordinates */
    intersection->ray = line;
    intersection->normal = dmnsn_vector_normalized(
      dmnsn_transform_normal(object->trans_inv, intersection->normal)
    );
    if (!intersection->object) {
      intersection->object = object;
    }

    dmnsn_assert(!isnan(intersection->t), "Intersection point is NaN.");
    dmnsn_assert(!dmnsn_vector_isnan(intersection->normal),
                 "Intersection normal is NaN.");

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
  return object->inside_fn(object, point);
}
