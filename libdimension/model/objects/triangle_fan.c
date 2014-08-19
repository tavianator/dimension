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

#include "internal.h"
#include "dimension/model.h"

/// Triangle fan type.
typedef struct {
  dmnsn_object object;
  size_t ncoeffs;
  double coeffs[][6];
} dmnsn_triangle_fan;

/// Change basis from one triangle to the next.
static inline dmnsn_vector
dmnsn_change_basis(const double coeffs[6], dmnsn_vector v)
{
  return dmnsn_new_vector(
    coeffs[0]*v.x + coeffs[1]*v.z + v.y,
    coeffs[2]*v.x + coeffs[3]*v.z,
    coeffs[4]*v.x + coeffs[5]*v.z
  );
}

/// Change basis from one triangle to the next for a normal vector.
static inline dmnsn_vector
dmnsn_change_normal_basis(const double coeffs[6], dmnsn_vector n)
{
  return dmnsn_new_vector(
    coeffs[0]*n.x + coeffs[2]*n.y + coeffs[4]*n.z,
    n.x,
    coeffs[1]*n.x + coeffs[3]*n.y + coeffs[5]*n.z
  );
}

/// Change basis from one triangle to the next for a ray
static inline dmnsn_ray
dmnsn_change_ray_basis(const double coeffs[6], dmnsn_ray l)
{
  return dmnsn_new_ray(dmnsn_change_basis(coeffs, l.x0), dmnsn_change_basis(coeffs, l.n));
}

/// Store the compressed incremental matrix.
static inline void
dmnsn_compress_coeffs(double coeffs[6], dmnsn_matrix incremental)
{
  coeffs[0] = incremental.n[0][0];
  coeffs[1] = incremental.n[0][2];
  coeffs[2] = incremental.n[1][0];
  coeffs[3] = incremental.n[1][2];
  coeffs[4] = incremental.n[2][0];
  coeffs[5] = incremental.n[2][2];
}

/// Decompress the incremental matrix.
static inline dmnsn_matrix
dmnsn_decompress_coeffs(const double coeffs[6])
{
  dmnsn_matrix incremental = dmnsn_new_matrix(
    coeffs[0], 1.0, coeffs[1], 0.0,
    coeffs[2], 0.0, coeffs[3], 0.0,
    coeffs[4], 0.0, coeffs[5], 0.0
  );
  return incremental;
}

/// Make a change-of-basis matrix for a triangle.
static inline dmnsn_matrix
dmnsn_triangle_basis(dmnsn_vector a, dmnsn_vector ab, dmnsn_vector ac)
{
  dmnsn_vector normal = dmnsn_vector_cross(ab, ac);
  return dmnsn_new_matrix4(ab, ac, normal, a);
}

/// Optimized ray/triangle intersection test.
static inline bool
dmnsn_ray_triangle_intersection(dmnsn_ray l, double *t, double *u, double *v)
{
  *t = -l.x0.z/l.n.z;
  *u = l.x0.x + (*t)*l.n.x;
  *v = l.x0.y + (*t)*l.n.y;
  return *t >= 0.0 && *u >= 0.0 && *v >= 0.0 && *u + *v <= 1.0;
}

/// Triangle fan intersection callback.
DMNSN_HOT static bool
dmnsn_triangle_fan_intersection_fn(const dmnsn_object *object, dmnsn_ray l, dmnsn_intersection *intersection)
{
  const dmnsn_triangle_fan *fan = (const dmnsn_triangle_fan *)object;

  double t, u, v;

  double best_t = INFINITY;
  if (dmnsn_ray_triangle_intersection(l, &t, &u, &v)) {
    best_t = t;
  }

  dmnsn_vector normal = dmnsn_z;
  dmnsn_vector best_normal = normal;

  for (size_t i = 0; i < fan->ncoeffs; ++i) {
    const double *coeffs = fan->coeffs[i];
    l = dmnsn_change_ray_basis(coeffs, l);
    normal = dmnsn_change_normal_basis(coeffs, normal);

    if (dmnsn_ray_triangle_intersection(l, &t, &u, &v) && t < best_t) {
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

/// Triangle fan inside callback.
static bool
dmnsn_triangle_fan_inside_fn(const dmnsn_object *object, dmnsn_vector point)
{
  return false;
}

/// Computes the bounding box for the first triangle
static inline dmnsn_aabb
dmnsn_bound_first_triangle(dmnsn_matrix trans)
{
  dmnsn_vector a = dmnsn_transform_point(trans, dmnsn_zero);
  dmnsn_vector b = dmnsn_transform_point(trans, dmnsn_x);
  dmnsn_vector c = dmnsn_transform_point(trans, dmnsn_y);

  dmnsn_aabb box = dmnsn_new_aabb(a, a);
  box = dmnsn_aabb_swallow(box, b);
  box = dmnsn_aabb_swallow(box, c);

  return box;
}

/// Triangle fan bounding callback.
static dmnsn_aabb
dmnsn_triangle_fan_bounding_fn(const dmnsn_object *object, dmnsn_matrix trans)
{
  const dmnsn_triangle_fan *fan = (const dmnsn_triangle_fan *)object;

  dmnsn_aabb box = dmnsn_bound_first_triangle(trans);

  for (size_t i = 0; i < fan->ncoeffs; ++i) {
    dmnsn_matrix incremental = dmnsn_decompress_coeffs(fan->coeffs[i]);
    trans = dmnsn_matrix_mul(trans, dmnsn_matrix_inverse(incremental));
    dmnsn_vector vertex = dmnsn_transform_point(trans, dmnsn_y);
    box = dmnsn_aabb_swallow(box, vertex);
  }

  return box;
}

/// Triangle fan vtable.
static dmnsn_object_vtable dmnsn_triangle_fan_vtable = {
  .intersection_fn = dmnsn_triangle_fan_intersection_fn,
  .inside_fn = dmnsn_triangle_fan_inside_fn,
  .bounding_fn = dmnsn_triangle_fan_bounding_fn,
};

/// Smooth triangle fan type.
typedef struct dmnsn_smooth_triangle_fan {
  dmnsn_object object;
  dmnsn_vector na, nab, nac;
  size_t ncoeffs;
  double coeffs[][9]; ///< 0-6 is same as dmnsn_triangle_fan, 6-9 is the normal
} dmnsn_smooth_triangle_fan;

/// Smooth triangle fan intersection callback.
DMNSN_HOT static bool
dmnsn_smooth_triangle_fan_intersection_fn(const dmnsn_object *object, dmnsn_ray l, dmnsn_intersection *intersection)
{
  const dmnsn_smooth_triangle_fan *fan = (const dmnsn_smooth_triangle_fan *)object;

  dmnsn_vector nab = fan->nab;
  dmnsn_vector nac = fan->nac;

  double t, u, v;

  double best_t = INFINITY;
  dmnsn_vector best_normal;
  if (dmnsn_ray_triangle_intersection(l, &t, &u, &v)) {
    best_t = t;
    best_normal = dmnsn_vector_add(dmnsn_vector_mul(u, nab), dmnsn_vector_mul(v, nac));
  }

  for (size_t i = 0; i < fan->ncoeffs; ++i) {
    const double *coeffs = fan->coeffs[i];
    l = dmnsn_change_ray_basis(coeffs, l);
    nab = nac;
    nac = dmnsn_new_vector(coeffs[6], coeffs[7], coeffs[8]);

    if (dmnsn_ray_triangle_intersection(l, &t, &u, &v) && t < best_t) {
      best_t = t;
      best_normal = dmnsn_vector_add(dmnsn_vector_mul(u, nab), dmnsn_vector_mul(v, nac));
    }
  }

  if (!isinf(best_t)) {
    intersection->t = t;
    intersection->normal = dmnsn_vector_add(fan->na, best_normal);
    return true;
  }

  return false;
}

/// Smooth triangle fan bounding callback.
static dmnsn_aabb
dmnsn_smooth_triangle_fan_bounding_fn(const dmnsn_object *object, dmnsn_matrix trans)
{
  const dmnsn_smooth_triangle_fan *fan = (const dmnsn_smooth_triangle_fan *)object;

  dmnsn_aabb box = dmnsn_bound_first_triangle(trans);

  for (size_t i = 0; i < fan->ncoeffs; ++i) {
    dmnsn_matrix incremental = dmnsn_decompress_coeffs(fan->coeffs[i]);
    trans = dmnsn_matrix_mul(trans, dmnsn_matrix_inverse(incremental));
    dmnsn_vector vertex = dmnsn_transform_point(trans, dmnsn_y);
    box = dmnsn_aabb_swallow(box, vertex);
  }

  return box;
}

/// Smooth triangle fan vtable.
static dmnsn_object_vtable dmnsn_smooth_triangle_fan_vtable = {
  .intersection_fn = dmnsn_smooth_triangle_fan_intersection_fn,
  .inside_fn = dmnsn_triangle_fan_inside_fn,
  .bounding_fn = dmnsn_smooth_triangle_fan_bounding_fn,
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

  // Compute the initial matrix and the coefficients
  dmnsn_vector a = vertices[0];
  dmnsn_vector ab = dmnsn_vector_sub(vertices[1], a);
  dmnsn_vector ac = dmnsn_vector_sub(vertices[2], a);
  dmnsn_matrix P = dmnsn_triangle_basis(a, ab, ac);
  object->intrinsic_trans = P;

  for (size_t i = 0; i < ncoeffs; ++i) {
    ab = ac;
    ac = dmnsn_vector_sub(vertices[i + 3], a);

    dmnsn_matrix newP = dmnsn_triangle_basis(a, ab, ac);
    dmnsn_matrix incremental = dmnsn_matrix_mul(dmnsn_matrix_inverse(newP), P);
    dmnsn_compress_coeffs(fan->coeffs[i], incremental);

    P = newP;
  }

  return object;
}

dmnsn_object *
dmnsn_new_smooth_triangle_fan(dmnsn_pool *pool, dmnsn_vector vertices[], dmnsn_vector normals[], size_t nvertices)
{
  dmnsn_assert(nvertices >= 3, "Not enough vertices for one triangle");

  size_t ncoeffs = nvertices - 3;
  dmnsn_smooth_triangle_fan *fan = dmnsn_palloc(pool, sizeof(dmnsn_smooth_triangle_fan) + ncoeffs*sizeof(double[9]));
  fan->ncoeffs = ncoeffs;

  dmnsn_object *object = &fan->object;
  dmnsn_init_object(object);
  object->vtable = &dmnsn_smooth_triangle_fan_vtable;

  // Compute the initial matrix
  dmnsn_vector a = vertices[0];
  dmnsn_vector ab = dmnsn_vector_sub(vertices[1], a);
  dmnsn_vector ac = dmnsn_vector_sub(vertices[2], a);
  dmnsn_matrix P = dmnsn_triangle_basis(a, ab, ac);
  dmnsn_matrix Pabc = P;
  object->intrinsic_trans = P;

  // Transform the first three normals
  dmnsn_vector na = dmnsn_vector_normalized(dmnsn_transform_normal(P, normals[0]));
  dmnsn_vector nb = dmnsn_vector_normalized(dmnsn_transform_normal(P, normals[1]));
  dmnsn_vector nc = dmnsn_vector_normalized(dmnsn_transform_normal(P, normals[2]));
  fan->na = na;
  fan->nab = dmnsn_vector_sub(nb, na);
  fan->nac = dmnsn_vector_sub(nc, na);

  // Compute the coefficients
  for (size_t i = 0; i < ncoeffs; ++i) {
    ab = ac;
    ac = dmnsn_vector_sub(vertices[i + 3], a);

    dmnsn_matrix newP = dmnsn_triangle_basis(a, ab, ac);
    dmnsn_matrix incremental = dmnsn_matrix_mul(dmnsn_matrix_inverse(newP), P);
    double *coeffs = fan->coeffs[i];
    dmnsn_compress_coeffs(coeffs, incremental);

    nc = dmnsn_vector_normalized(dmnsn_transform_normal(Pabc, normals[i + 3]));
    dmnsn_vector nac = dmnsn_vector_sub(nc, na);
    coeffs[6] = nac.x;
    coeffs[7] = nac.y;
    coeffs[8] = nac.z;

    P = newP;
  }

  return object;
}
