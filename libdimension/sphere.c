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

#include "dimension.h"
#include <math.h>   /* For sqrt */

/*
 * Sphere
 */

/* Sphere object callbacks */

static bool dmnsn_sphere_intersection_fn(const dmnsn_object *sphere,
                                         dmnsn_line line,
                                         dmnsn_intersection *intersection);
static bool dmnsn_sphere_inside_fn(const dmnsn_object *sphere,
                                   dmnsn_vector point);

/* Allocate a new sphere */
dmnsn_object *
dmnsn_new_sphere()
{
  dmnsn_object *sphere = dmnsn_new_object();
  sphere->intersection_fn  = &dmnsn_sphere_intersection_fn;
  sphere->inside_fn        = &dmnsn_sphere_inside_fn;
  sphere->bounding_box.min = dmnsn_new_vector(-1.0, -1.0, -1.0);
  sphere->bounding_box.max = dmnsn_new_vector(1.0, 1.0, 1.0);
  return sphere;
}

/* Returns the closest intersection of `line' with `sphere' */
static bool
dmnsn_sphere_intersection_fn(const dmnsn_object *sphere, dmnsn_line line,
                             dmnsn_intersection *intersection)
{
  dmnsn_line l = dmnsn_matrix_line_mul(sphere->trans_inv, line);
 
  /* Solve (x0 + nx*t)^2 + (y0 + ny*t)^2 + (z0 + nz*t)^2 == 1 */
  double a, b, c, t;
  a = l.n.x*l.n.x + l.n.y*l.n.y + l.n.z*l.n.z;
  b = 2.0*(l.n.x*l.x0.x + l.n.y*l.x0.y + l.n.z*l.x0.z);
  c = l.x0.x*l.x0.x + l.x0.y*l.x0.y + l.x0.z*l.x0.z - 1.0;

  if (b*b - 4.0*a*c >= 0) {
    t = (-b - sqrt(b*b - 4.0*a*c))/(2*a);
    if (t < 0.0) {
      t = (-b + sqrt(b*b - 4.0*a*c))/(2*a);
    }

    if (t >= 0.0) {
      intersection->ray      = line;
      intersection->t        = t;
      intersection->normal   = dmnsn_matrix_normal_mul(sphere->trans,
                                                       dmnsn_line_point(l, t));
      intersection->texture  = sphere->texture;
      intersection->interior = sphere->interior;
      return true;
    }
  }

  return false;
}

/* Return whether a point is inside a sphere (x**2 + y**2 + z**2 < 1.0) */
static bool
dmnsn_sphere_inside_fn(const dmnsn_object *sphere, dmnsn_vector point)
{
  point = dmnsn_matrix_vector_mul(sphere->trans_inv, point);
  return point.x*point.x + point.y*point.y + point.z*point.z < 1.0;
}
