/*************************************************************************
 * Copyright (C) 2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Triangle fans.  See
 * http://tavianator.com/2014/05/a-beautiful-raymesh-intersection-algorithm/
 * for a description of the intersection algorithm.
 */

#include "dimension.h"

/** Triangle fan type. */
typedef struct {
  dmnsn_object object;
  size_t ncoeffs;
  double coeffs[][6];
} dmnsn_triangle_fan;

/** Change basis from one triangle to the next. */
static inline dmnsn_vector
dmnsn_change_basis(const double coeffs[6], dmnsn_vector v)
{
  return dmnsn_new_vector(
    coeffs[0]*v.x + coeffs[1]*v.z + v.y,
    coeffs[2]*v.x + coeffs[3]*v.z,
    coeffs[4]*v.x + coeffs[5]*v.z
  );
}

/** Change basis from one triangle to the next for a normal vector. */
static inline dmnsn_vector
dmnsn_change_normal_basis(const double coeffs[6], dmnsn_vector n)
{
  return dmnsn_new_vector(
    coeffs[0]*n.x + coeffs[2]*n.y + coeffs[4]*n.z,
    n.x,
    coeffs[1]*n.x + coeffs[3]*n.y + coeffs[5]*n.z
  );
}

/** Triangle intersection callback. */
static bool
dmnsn_triangle_fan_intersection_fn(const dmnsn_object *object, dmnsn_line l, dmnsn_intersection *intersection)
{
  const dmnsn_triangle_fan *fan = (const dmnsn_triangle_fan *)object;

  double t = -l.x0.z/l.n.z;
  double u = l.x0.x + t*l.n.x;
  double v = l.x0.y + t*l.n.y;

  double best_t = INFINITY;
  if (t >= 0.0 && u >= 0.0 && v >= 0.0 && u + v <= 1.0) {
    best_t = t;
  }

  dmnsn_vector normal = dmnsn_z;
  dmnsn_vector best_normal = normal;

  for (size_t i = 0; i < fan->ncoeffs; ++i) {
    const double *coeffs = fan->coeffs[i];
    l = dmnsn_new_line(dmnsn_change_basis(coeffs, l.x0), dmnsn_change_basis(coeffs, l.n));
    normal = dmnsn_change_normal_basis(coeffs, normal);

    t = -l.x0.z/l.n.z;
    u = l.x0.x + t*l.n.x;
    v = l.x0.y + t*l.n.y;

    if (t >= 0.0 && t < best_t && u >= 0.0 && v >= 0.0 && u + v <= 1.0) {
      best_t = t;
      best_normal = normal;
    }
  }

  if (!isinf(best_t)) {
    intersection->t = t;
    intersection->normal = best_normal;
    return true;
  }

  return false;
}

/** Triangle fan inside callback. */
static bool
dmnsn_triangle_fan_inside_fn(const dmnsn_object *object, dmnsn_vector point)
{
  return false;
}

/** Triangle fan bounding callback. */
static dmnsn_bounding_box
dmnsn_triangle_fan_bounding_fn(const dmnsn_object *object, dmnsn_matrix trans)
{
  const dmnsn_triangle_fan *fan = (const dmnsn_triangle_fan *)object;

  dmnsn_vector a = dmnsn_transform_point(trans, dmnsn_zero);
  dmnsn_vector b = dmnsn_transform_point(trans, dmnsn_x);
  dmnsn_vector c = dmnsn_transform_point(trans, dmnsn_y);

  dmnsn_bounding_box box = dmnsn_new_bounding_box(a, a);
  box = dmnsn_bounding_box_swallow(box, b);
  box = dmnsn_bounding_box_swallow(box, c);

  dmnsn_matrix P = trans;

  for (size_t i = 0; i < fan->ncoeffs; ++i) {
    const double *coeffs = fan->coeffs[i];
    dmnsn_matrix incremental = dmnsn_new_matrix(coeffs[0], 1.0, coeffs[1], 0.0,
                                                coeffs[2], 0.0, coeffs[3], 0.0,
                                                coeffs[4], 0.0, coeffs[5], 0.0);
    P = dmnsn_matrix_mul(P, dmnsn_matrix_inverse(incremental));
    dmnsn_vector d = dmnsn_transform_point(P, dmnsn_y);
    box = dmnsn_bounding_box_swallow(box, d);
  }

  return box;
}

/** Triangle fan vtable. */
static dmnsn_object_vtable dmnsn_triangle_fan_vtable = {
  .intersection_fn = dmnsn_triangle_fan_intersection_fn,
  .inside_fn = dmnsn_triangle_fan_inside_fn,
  .bounding_fn = dmnsn_triangle_fan_bounding_fn,
};

dmnsn_object *
dmnsn_new_triangle_fan(dmnsn_pool *pool, dmnsn_vector vertices[], size_t nvertices)
{
  dmnsn_assert(nvertices >= 3, "Not enough vertices for one triangle");

  size_t ncoeffs = nvertices - 3;
  dmnsn_triangle_fan *fan = dmnsn_palloc(pool, sizeof(dmnsn_triangle_fan) + ncoeffs*sizeof(double[6]));
  fan->ncoeffs = ncoeffs;

  dmnsn_object *object = &fan->object;
  dmnsn_init_object(object);
  object->vtable = &dmnsn_triangle_fan_vtable;

  /* Compute the initial matrix and the coefficients */
  dmnsn_vector a = vertices[0];
  dmnsn_vector ab = dmnsn_vector_sub(vertices[1], a);
  dmnsn_vector ac = dmnsn_vector_sub(vertices[2], a);
  dmnsn_vector normal = dmnsn_vector_cross(ab, ac);
  dmnsn_matrix P = dmnsn_new_matrix4(ab, ac, normal, a);
  object->intrinsic_trans = P;

  for (size_t i = 0; i < ncoeffs; ++i) {
    ab = ac;
    ac = dmnsn_vector_sub(vertices[i + 3], a);
    normal = dmnsn_vector_cross(ab, ac);

    dmnsn_matrix newP = dmnsn_new_matrix4(ab, ac, normal, a);
    dmnsn_matrix incremental = dmnsn_matrix_mul(dmnsn_matrix_inverse(newP), P);
    double *coeffs = fan->coeffs[i];
    coeffs[0] = incremental.n[0][0];
    coeffs[1] = incremental.n[0][2];
    coeffs[2] = incremental.n[1][0];
    coeffs[3] = incremental.n[1][2];
    coeffs[4] = incremental.n[2][0];
    coeffs[5] = incremental.n[2][2];

    P = newP;
  }

  return object;
}
