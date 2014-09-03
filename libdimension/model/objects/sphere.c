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
 * Spheres.
 */

#include "internal.h"
#include "internal/polynomial.h"
#include "dimension/model.h"

/// Sphere intersection callback.
static bool
dmnsn_sphere_intersection_fn(const dmnsn_object *sphere, dmnsn_ray l, dmnsn_intersection *intersection)
{
  // Solve (x0 + nx*t)^2 + (y0 + ny*t)^2 + (z0 + nz*t)^2 == 1
  double poly[3], x[2];
  poly[2] = dmnsn_vector_dot(l.n, l.n);
  poly[1] = 2.0*dmnsn_vector_dot(l.n, l.x0);
  poly[0] = dmnsn_vector_dot(l.x0, l.x0) - 1.0;

  size_t n = dmnsn_polynomial_solve(poly, 2, x);
  if (n == 0) {
    return false;
  }

  double t = x[0];
  // Optimize for the case where we're outside the sphere
  if (dmnsn_likely(n == 2)) {
    t = dmnsn_min(t, x[1]);
  }

  intersection->t = t;
  intersection->normal = dmnsn_ray_point(l, t);
  return true;
}

/// Sphere inside callback.
static bool
dmnsn_sphere_inside_fn(const dmnsn_object *sphere, dmnsn_vector point)
{
  return point.X*point.X + point.Y*point.Y + point.Z*point.Z < 1.0;
}

/// Helper for sphere bounding box calculation.
static inline double
dmnsn_implicit_dot(const double row[4])
{
  double ret = 0.0;
  for (int i = 0; i < 3; ++i) {
    ret += row[i]*row[i];
  }
  return ret;
}

/// Sphere bounding callback.
static dmnsn_aabb
dmnsn_sphere_bounding_fn(const dmnsn_object *object, dmnsn_matrix trans)
{
  // Get a tight bound using the quadric representation of a sphere.  For
  // details, see
  // http://tavianator.com/2014/06/exact-bounding-boxes-for-spheres-ellipsoids

  dmnsn_aabb box;

  double cx = trans.n[0][3];
  double dx = sqrt(dmnsn_implicit_dot(trans.n[0]));
  box.min.X = cx - dx;
  box.max.X = cx + dx;

  double cy = trans.n[1][3];
  double dy = sqrt(dmnsn_implicit_dot(trans.n[1]));
  box.min.Y = cy - dy;
  box.max.Y = cy + dy;

  double cz = trans.n[2][3];
  double dz = sqrt(dmnsn_implicit_dot(trans.n[2]));
  box.min.Z = cz - dz;
  box.max.Z = cz + dz;

  return box;
}

/// Sphere vtable.
static const dmnsn_object_vtable dmnsn_sphere_vtable = {
  .intersection_fn = dmnsn_sphere_intersection_fn,
  .inside_fn = dmnsn_sphere_inside_fn,
  .bounding_fn = dmnsn_sphere_bounding_fn,
};

dmnsn_object *
dmnsn_new_sphere(dmnsn_pool *pool)
{
  dmnsn_object *sphere = dmnsn_new_object(pool);
  sphere->vtable = &dmnsn_sphere_vtable;
  return sphere;
}
