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
 * Planes.
 */

#include "dimension/model.h"
#include <math.h>
#include <stdlib.h>

/// Plane type.
typedef struct {
  dmnsn_object object;
  dmnsn_vector normal;
} dmnsn_plane;

/// Returns the closest intersection of `ray' with `plane'.
static bool
dmnsn_plane_intersection_fn(const dmnsn_object *object, dmnsn_ray ray,
                            dmnsn_intersection *intersection)
{
  const dmnsn_plane *plane = (const dmnsn_plane *)object;
  dmnsn_vector normal = plane->normal;

  double den = dmnsn_vector_dot(ray.n, normal);
  if (den != 0.0) {
    double t = -dmnsn_vector_dot(ray.x0, normal)/den;
    if (t >= 0.0) {
      intersection->t      = t;
      intersection->normal = normal;
      return true;
    }
  }
  return false;
}

/// Return whether a point is inside a plane.
static bool
dmnsn_plane_inside_fn(const dmnsn_object *object, dmnsn_vector point)
{
  const dmnsn_plane *plane = (const dmnsn_plane *)object;
  return dmnsn_vector_dot(point, plane->normal) < 0.0;
}

/// Plane bounding callback.
static dmnsn_aabb
dmnsn_plane_bounding_fn(const dmnsn_object *object, dmnsn_matrix trans)
{
  return dmnsn_infinite_aabb();
}

/// Plane vtable.
static const dmnsn_object_vtable dmnsn_plane_vtable = {
  .intersection_fn = dmnsn_plane_intersection_fn,
  .inside_fn = dmnsn_plane_inside_fn,
  .bounding_fn = dmnsn_plane_bounding_fn,
};

dmnsn_object *
dmnsn_new_plane(dmnsn_pool *pool, dmnsn_vector normal)
{
  dmnsn_plane *plane = DMNSN_PALLOC(pool, dmnsn_plane);
  plane->normal = normal;

  dmnsn_object *object = &plane->object;
  dmnsn_init_object(object);
  object->vtable = &dmnsn_plane_vtable;
  return object;
}
