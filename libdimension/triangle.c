/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Triangles.
 */

#include "dimension.h"

typedef struct dmnsn_triangle_payload {
  dmnsn_vector a, ab, ac, normal;
} dmnsn_triangle_payload;

/** Triangle intersection callback. */
static bool
dmnsn_triangle_intersection_fn(const dmnsn_object *triangle, dmnsn_line l,
                               dmnsn_intersection *intersection)
{
  const dmnsn_triangle_payload *payload = triangle->ptr;

  double den = -dmnsn_vector_dot(l.n, payload->normal);
  dmnsn_vector ax0 = dmnsn_vector_sub(l.x0, payload->a);
  double t = dmnsn_vector_dot(ax0, payload->normal)/den;
  double u = -dmnsn_vector_dot(l.n, dmnsn_vector_cross(ax0, payload->ac))/den;
  double v = -dmnsn_vector_dot(l.n, dmnsn_vector_cross(payload->ab, ax0))/den;
  if (t >= 0.0 && u >= 0.0 && v >= 0.0 && u + v <= 1.0) {
    intersection->t      = t;
    intersection->normal = payload->normal;
    return true;
  }

  return false;
}

/** Triangle inside callback. */
static bool
dmnsn_triangle_inside_fn(const dmnsn_object *triangle, dmnsn_vector point)
{
  return false;
}

/* Allocate a new triangle */
dmnsn_object *
dmnsn_new_triangle(dmnsn_vector a, dmnsn_vector b, dmnsn_vector c)
{
  dmnsn_object *triangle = dmnsn_new_object();

  dmnsn_triangle_payload *payload =
    dmnsn_malloc(sizeof(dmnsn_triangle_payload));
  payload->a      = a;
  payload->ab     = dmnsn_vector_sub(b, a);
  payload->ac     = dmnsn_vector_sub(c, a);
  payload->normal = dmnsn_vector_cross(payload->ab, payload->ac);

  triangle->ptr              = payload;
  triangle->intersection_fn  = dmnsn_triangle_intersection_fn;
  triangle->inside_fn        = dmnsn_triangle_inside_fn;
  triangle->free_fn          = dmnsn_free;
  triangle->bounding_box.min = dmnsn_vector_min(dmnsn_vector_min(a, b), c);
  triangle->bounding_box.max = dmnsn_vector_max(dmnsn_vector_max(a, b), c);
  return triangle;
}
