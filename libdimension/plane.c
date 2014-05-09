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

#include "dimension.h"
#include <math.h>
#include <stdlib.h>

/** Plane type. */
typedef struct {
  dmnsn_object object;
  dmnsn_vector normal;
} dmnsn_plane;

/* Plane object callbacks */

static bool dmnsn_plane_intersection_fn(const dmnsn_object *plane,
                                        dmnsn_line line,
                                        dmnsn_intersection *intersection);
static bool dmnsn_plane_inside_fn(const dmnsn_object *plane,
                                  dmnsn_vector point);

/* Allocate a new plane */
dmnsn_object *
dmnsn_new_plane(dmnsn_vector normal)
{
  dmnsn_plane *plane = DMNSN_MALLOC(dmnsn_plane);
  dmnsn_init_object(&plane->object);

  plane->object.intersection_fn = dmnsn_plane_intersection_fn;
  plane->object.inside_fn = dmnsn_plane_inside_fn;
  plane->object.bounding_box = dmnsn_infinite_bounding_box();
  plane->normal = normal;
  return &plane->object;
}

/* Returns the closest intersection of `line' with `plane' */
static bool
dmnsn_plane_intersection_fn(const dmnsn_object *object, dmnsn_line line,
                            dmnsn_intersection *intersection)
{
  const dmnsn_plane *plane = (const dmnsn_plane *)object;
  dmnsn_vector normal = plane->normal;

  double den = dmnsn_vector_dot(line.n, normal);
  if (den != 0.0) {
    double t = -dmnsn_vector_dot(line.x0, normal)/den;
    if (t >= 0.0) {
      intersection->t      = t;
      intersection->normal = normal;
      return true;
    }
  }
  return false;
}

/* Return whether a point is inside a plane */
static bool
dmnsn_plane_inside_fn(const dmnsn_object *object, dmnsn_vector point)
{
  const dmnsn_plane *plane = (const dmnsn_plane *)object;
  return dmnsn_vector_dot(point, plane->normal) < 0.0;
}
