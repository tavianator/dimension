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
 * Cubes.
 */

#include "dimension.h"
#include <math.h>

/** Intersection callback for a cube. */
static bool
dmnsn_cube_intersection_fn(const dmnsn_object *cube, dmnsn_line line,
                           dmnsn_intersection *intersection)
{
  /* Clip the given line against the X, Y, and Z slabs */

  dmnsn_vector nmin, nmax;
  double tmin, tmax;

  double tx1 = (-1.0 - line.x0.x)/line.n.x;
  double tx2 = (+1.0 - line.x0.x)/line.n.x;

  if (tx1 < tx2) {
    tmin = tx1;
    tmax = tx2;
    nmin = dmnsn_new_vector(-1.0, 0.0, 0.0);
    nmax = dmnsn_new_vector(+1.0, 0.0, 0.0);
  } else {
    tmin = tx2;
    tmax = tx1;
    nmin = dmnsn_new_vector(+1.0, 0.0, 0.0);
    nmax = dmnsn_new_vector(-1.0, 0.0, 0.0);
  }

  if (tmin > tmax)
    return false;

  double ty1 = (-1.0 - line.x0.y)/line.n.y;
  double ty2 = (+1.0 - line.x0.y)/line.n.y;

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

  double tz1 = (-1.0 - line.x0.z)/line.n.z;
  double tz2 = (+1.0 - line.x0.z)/line.n.z;

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

  if (tmin < 0.0) {
    tmin = tmax;
    nmin = nmax;
  }

  if (tmin >= 0.0) {
    intersection->t      = tmin;
    intersection->normal = nmin;
    return true;
  } else {
    return false;
  }
}

/** Inside callback for a cube. */
static bool
dmnsn_cube_inside_fn(const dmnsn_object *cube, dmnsn_vector point)
{
  return point.x > -1.0 && point.x < 1.0
      && point.y > -1.0 && point.y < 1.0
      && point.z > -1.0 && point.z < 1.0;
}

/** Cube vtable. */
static const dmnsn_object_vtable dmnsn_cube_vtable = {
  .intersection_fn = dmnsn_cube_intersection_fn,
  .inside_fn = dmnsn_cube_inside_fn,
};

/* Allocate a new cube object */
dmnsn_object *
dmnsn_new_cube(dmnsn_pool *pool)
{
  dmnsn_object *cube = dmnsn_new_object(pool);
  cube->vtable = &dmnsn_cube_vtable;
  cube->bounding_box.min = dmnsn_new_vector(-1.0, -1.0, -1.0);
  cube->bounding_box.max = dmnsn_new_vector(1.0, 1.0, 1.0);
  return cube;
}
