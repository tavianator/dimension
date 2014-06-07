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

#include "dimension-internal.h"

/// Sphere intersection callback.
static bool
dmnsn_sphere_intersection_fn(const dmnsn_object *sphere, dmnsn_line l,
                             dmnsn_intersection *intersection)
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
  intersection->normal = dmnsn_line_point(l, t);
  return true;
}

/// Sphere inside callback.
static bool
dmnsn_sphere_inside_fn(const dmnsn_object *sphere, dmnsn_vector point)
{
  return point.x*point.x + point.y*point.y + point.z*point.z < 1.0;
}

/// Sphere bounding callback.
static dmnsn_bounding_box
dmnsn_sphere_bounding_fn(const dmnsn_object *object, dmnsn_matrix trans)
{
  // Get a tight bound using the conic representation of a sphere:
  //
  //   S = [ 1  0  0  0 ]
  //       [ 0  1  0  0 ]
  //       [ 0  0  1  0 ]
  //       [ 0  0  0 -1 ].
  //
  // The surface is defined by
  //   p^T * S * p = 0,
  // and the tangent planes are defined by
  //   q * S^-1 * q^T = 0.
  // Note that S = S^-1.
  //
  // The symmetric matrix R, defined by
  //   R = M * S^-1 * M^T,
  // characterizes the tangent planes. Specifically,
  //   min.x = (R[0,3] - sqrt(R[0,3]^2 - R[0,0]*R[3,3]))/R[3,3]
  //   max.x = (R[0,3] + sqrt(R[0,3]^2 - R[0,0]*R[3,3]))/R[3,3]
  //   min.y = (R[1,3] - sqrt(R[1,3]^2 - R[1,1]*R[3,3]))/R[3,3]
  //   max.y = (R[1,3] + sqrt(R[1,3]^2 - R[1,1]*R[3,3]))/R[3,3]
  //   min.z = (R[2,3] - sqrt(R[2,3]^2 - R[2,2]*R[3,3]))/R[3,3]
  //   max.z = (R[2,3] + sqrt(R[2,3]^2 - R[2,2]*R[3,3]))/R[3,3]
  //
  // Unfortunately, we can't use dmnsn_matrix because the matrices are not
  // affine

  // MS = M * S^-1 = M * S
  // Last row is [ 0  0  0 -1 ] implicitly
  double MS[3][4];
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      MS[i][j] = trans.n[i][j];
    }
    MS[i][3] = -trans.n[i][3];
  }

  // R = MS * M^T
  // We only compute the upper triangular portion
  // R[3][3] is implicitly -1
  double R[4][4];
  for (int i = 0; i < 3; ++i) {
    for (int j = i; j < 3; ++j) {
      R[i][j] = 0.0;
      for (int k = 0; k < 4; ++k) {
        R[i][j] += MS[i][k]*trans.n[j][k];
      }
    }
    R[i][3] = MS[i][3];
  }

  dmnsn_bounding_box box;

  double dx = sqrt(R[0][3]*R[0][3] + R[0][0]);
  box.min.x = -R[0][3] - dx;
  box.max.x = -R[0][3] + dx;

  double dy = sqrt(R[1][3]*R[1][3] + R[1][1]);
  box.min.y = -R[1][3] - dy;
  box.max.y = -R[1][3] + dy;

  double dz = sqrt(R[2][3]*R[2][3] + R[2][2]);
  box.min.z = -R[2][3] - dz;
  box.max.z = -R[2][3] + dz;

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
