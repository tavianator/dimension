/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
  dmnsn_object *plane = dmnsn_new_object();

  dmnsn_vector *param = dmnsn_malloc(sizeof(dmnsn_vector));
  *param = normal;

  plane->intersection_fn  = &dmnsn_plane_intersection_fn;
  plane->inside_fn        = &dmnsn_plane_inside_fn;
  plane->free_fn          = &dmnsn_free;
  plane->bounding_box     = dmnsn_infinite_bounding_box();
  plane->ptr              = param;
  return plane;
}

/* Returns the closest intersection of `line' with `plane' */
static bool
dmnsn_plane_intersection_fn(const dmnsn_object *plane, dmnsn_line line,
                            dmnsn_intersection *intersection)
{
  dmnsn_vector *normal = plane->ptr;

  double den = dmnsn_vector_dot(line.n, *normal);
  if (den != 0.0) {
    double t = -dmnsn_vector_dot(line.x0, *normal)/den;
    if (t >= 0.0) {
      intersection->ray      = line;
      intersection->t        = t;
      intersection->normal   = *normal;
      intersection->texture  = plane->texture;
      intersection->interior = plane->interior;
      return true;
    }
  }
  return false;
}

/* Return whether a point is inside a plane */
static bool
dmnsn_plane_inside_fn(const dmnsn_object *plane, dmnsn_vector point)
{
  dmnsn_vector *normal = plane->ptr;
  return dmnsn_vector_dot(point, *normal) < 0.0;
}
