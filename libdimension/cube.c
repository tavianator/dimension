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

#include "dimension.h"
#include <math.h>

/*
 * Cube
 */

/* Cube callbacks */
static bool dmnsn_cube_intersection_fn(const dmnsn_object *cube,
                                       dmnsn_line line,
                                       dmnsn_intersection *intersection);
static bool dmnsn_cube_inside_fn(const dmnsn_object *cube,
                                 dmnsn_vector point);

/* Allocate a new cube object */
dmnsn_object *
dmnsn_new_cube()
{
  dmnsn_object *cube = dmnsn_new_object();
  cube->intersection_fn  = &dmnsn_cube_intersection_fn;
  cube->inside_fn        = &dmnsn_cube_inside_fn;
  cube->bounding_box.min = dmnsn_new_vector(-1.0, -1.0, -1.0);
  cube->bounding_box.max = dmnsn_new_vector(1.0, 1.0, 1.0);
  return cube;
}

/* Intersections callback for a cube */
static bool
dmnsn_cube_intersection_fn(const dmnsn_object *cube, dmnsn_line line,
                           dmnsn_intersection *intersection)
{
  /* Clip the given line against the X, Y, and Z slabs */

  dmnsn_line line_trans = dmnsn_transform_line(cube->trans_inv, line);

  dmnsn_vector nmin, nmax;
  double tmin = -INFINITY, tmax = INFINITY;

  if (line_trans.n.x != 0.0) {
    double tx1 = (-1.0 - line_trans.x0.x)/line_trans.n.x;
    double tx2 = (+1.0 - line_trans.x0.x)/line_trans.n.x;

    if (tx1 < tx2) {
      if (tx1 > tmin) {
        tmin = tx1;
        nmin = dmnsn_new_vector(-1.0, 0.0, 0.0);
      }
      if (tx2 < tmax) {
        tmax = tx2;
        nmax = dmnsn_new_vector(+1.0, 0.0, 0.0);
      }
    } else {
      if (tx2 > tmin) {
        tmin = tx2;
        nmin = dmnsn_new_vector(+1.0, 0.0, 0.0);
      }
      if (tx1 < tmax) {
        tmax = tx1;
        nmax = dmnsn_new_vector(-1.0, 0.0, 0.0);
      }
    }

    if (tmin > tmax)
      return false;
  } else {
    if (line_trans.x0.x < -1.0 || line_trans.x0.x > 1.0)
      return false;
  }

  if (line_trans.n.y != 0.0) {
    double ty1 = (-1.0 - line_trans.x0.y)/line_trans.n.y;
    double ty2 = (+1.0 - line_trans.x0.y)/line_trans.n.y;

    if (ty1 < ty2) {
      if (ty1 > tmin) {
        tmin = ty1;
        nmin = dmnsn_new_vector(0.0, -1.0, 0.0);
      }
      if (ty2 < tmax) {
        tmax = ty2;
        nmax = dmnsn_new_vector(0.0, +1.0, 0.0);
      }
    } else {
      if (ty2 > tmin) {
        tmin = ty2;
        nmin = dmnsn_new_vector(0.0, +1.0, 0.0);
      }
      if (ty1 < tmax) {
        tmax = ty1;
        nmax = dmnsn_new_vector(0.0, -1.0, 0.0);
      }
    }

    if (tmin > tmax)
      return false;
  } else {
    if (line_trans.x0.y < -1.0 || line_trans.x0.y > 1.0)
      return false;
  }

  if (line_trans.n.z != 0.0) {
    double tz1 = (-1.0 - line_trans.x0.z)/line_trans.n.z;
    double tz2 = (+1.0 - line_trans.x0.z)/line_trans.n.z;

    if (tz1 < tz2) {
      if (tz1 > tmin) {
        tmin = tz1;
        nmin = dmnsn_new_vector(0.0, 0.0, -1.0);
      }
      if (tz2 < tmax) {
        tmax = tz2;
        nmax = dmnsn_new_vector(0.0, 0.0, +1.0);
      }
    } else {
      if (tz2 > tmin) {
        tmin = tz2;
        nmin = dmnsn_new_vector(0.0, 0.0, +1.0);
      }
      if (tz1 < tmax) {
        tmax = tz1;
        nmax = dmnsn_new_vector(0.0, 0.0, -1.0);
      }
    }

    if (tmin > tmax)
      return false;
  } else {
    if (line_trans.x0.z < -1.0 || line_trans.x0.z > 1.0)
      return false;
  }

  if (tmin < 0.0) {
    tmin = tmax;
    nmin = nmax;
  }

  if (tmin >= 0.0) {
    intersection->ray      = line;
    intersection->t        = tmin;
    intersection->normal   = dmnsn_transform_normal(cube->trans, nmin);
    intersection->texture  = cube->texture;
    intersection->interior = cube->interior;
    return true;
  } else {
    return false;
  }
}

/* Inside callback for a cube */
static bool
dmnsn_cube_inside_fn(const dmnsn_object *cube, dmnsn_vector point)
{
  point = dmnsn_transform_vector(cube->trans_inv, point);
  return point.x > -1.0 && point.x < 1.0
      && point.y > -1.0 && point.y < 1.0
      && point.z > -1.0 && point.z < 1.0;
}
