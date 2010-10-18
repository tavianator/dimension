/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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

/*
 * Objects.
 */

#ifndef DIMENSION_OBJECT_H
#define DIMENSION_OBJECT_H

#include <stdbool.h>

/* A type to represent a ray-object intersection */
typedef struct dmnsn_intersection {
  /* The ray and point which intersected */
  dmnsn_line ray;
  double t;

  /* The surface normal at the intersection point */
  dmnsn_vector normal;

  /* The object properties at the intersection point */
  const dmnsn_texture  *texture;
  const dmnsn_interior *interior;
} dmnsn_intersection;

/* Forward-declare dmnsn_object */
typedef struct dmnsn_object dmnsn_object;

/* Object callback types */

typedef void dmnsn_object_init_fn(dmnsn_object *object);
typedef bool dmnsn_object_intersection_fn(const dmnsn_object *object,
                                          dmnsn_line line,
                                          dmnsn_intersection *intersection);
typedef bool dmnsn_object_inside_fn(const dmnsn_object *object,
                                    dmnsn_vector point);

/* dmnsn_object definition */
struct dmnsn_object {
  /* Surface properties */
  dmnsn_texture *texture;

  /* Interior properties */
  dmnsn_interior *interior;

  /* Transformation matrix */
  dmnsn_matrix trans, trans_inv;

  /* Bounding box */
  dmnsn_bounding_box bounding_box;

  /* Child objects */
  dmnsn_array *children;

  /* Callback functions */
  dmnsn_object_init_fn         *init_fn;
  dmnsn_object_intersection_fn *intersection_fn;
  dmnsn_object_inside_fn       *inside_fn;
  dmnsn_free_fn                *free_fn;

  /* Generic pointer for object info */
  void *ptr;
};

/* Allocate a dummy object */
dmnsn_object *dmnsn_new_object(void);
/* Free an object */
void dmnsn_delete_object(dmnsn_object *object);

void dmnsn_object_init(dmnsn_object *object);

/* Useful function to transform a normal vector */
DMNSN_INLINE dmnsn_vector
dmnsn_transform_normal(dmnsn_matrix trans, dmnsn_vector normal)
{
  return dmnsn_vector_normalize(
    dmnsn_vector_sub(
      dmnsn_transform_vector(trans, normal),
      /* Optimized form of dmnsn_transform_vector(trans, dmnsn_zero) */
      dmnsn_vector_div(
        dmnsn_new_vector(trans.n[0][3], trans.n[1][3], trans.n[2][3]),
        trans.n[3][3]
      )
    )
  );
}

#endif /* DIMENSION_OBJECT_H */
