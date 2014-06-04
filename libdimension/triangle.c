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
 * Triangles.  See
 * http://tavianator.com/2014/05/a-beautiful-raytriangle-intersection-method/
 * for a description of the intersection algorithm.
 */

#include "dimension.h"

/** Optimized ray/triangle intersection test. */
static inline bool
dmnsn_ray_triangle_intersection(dmnsn_line l, double *t, double *u, double *v)
{
  /* See the change of basis in dmnsn_triangle_basis() */
  *t = -l.x0.z/l.n.z;
  *u = l.x0.x + (*t)*l.n.x;
  *v = l.x0.y + (*t)*l.n.y;
  return *t >= 0.0 && *u >= 0.0 && *v >= 0.0 && *u + *v <= 1.0;
}

/** Triangle intersection callback. */
static bool
dmnsn_triangle_intersection_fn(const dmnsn_object *object, dmnsn_line l,
                               dmnsn_intersection *intersection)
{
  double t, u, v;
  if (dmnsn_ray_triangle_intersection(l, &t, &u, &v)) {
    intersection->t = t;
    intersection->normal = dmnsn_z;
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

/** Smooth triangle type. */
typedef struct {
  dmnsn_object object;
  dmnsn_vector na, nab, nac;
} dmnsn_smooth_triangle;

/** Smooth triangle intersection callback. */
static bool
dmnsn_smooth_triangle_intersection_fn(const dmnsn_object *object, dmnsn_line l,
                                      dmnsn_intersection *intersection)
{
  const dmnsn_smooth_triangle *triangle = (const dmnsn_smooth_triangle *)object;

  double t, u, v;
  if (dmnsn_ray_triangle_intersection(l, &t, &u, &v)) {
    intersection->t = t;
    intersection->normal = dmnsn_vector_add(
      triangle->na,
      dmnsn_vector_add(
        dmnsn_vector_mul(u, triangle->nab),
        dmnsn_vector_mul(v, triangle->nac)
      )
    );
    return true;
  }

  return false;
}

/** Make a change-of-basis matrix. */
static inline dmnsn_matrix
dmnsn_triangle_basis(dmnsn_vector vertices[3])
{
  /*
   * The new vector space has corners at <0, 1, 0>, <0, 0, 1>, and 0,
   * corresponding to the basis (ab, ac, ab X ac).
   */
  dmnsn_vector ab = dmnsn_vector_sub(vertices[1], vertices[0]);
  dmnsn_vector ac = dmnsn_vector_sub(vertices[2], vertices[0]);
  dmnsn_vector normal = dmnsn_vector_cross(ab, ac);
  return dmnsn_new_matrix4(ab, ac, normal, vertices[0]);
}

dmnsn_object *
dmnsn_new_triangle(dmnsn_pool *pool, dmnsn_vector vertices[3])
{
  dmnsn_object *object = dmnsn_new_object(pool);
  object->intersection_fn = dmnsn_triangle_intersection_fn;
  object->inside_fn = dmnsn_triangle_inside_fn;
  object->bounding_box.min = dmnsn_zero;
  object->bounding_box.max = dmnsn_new_vector(1.0, 1.0, 0.0);
  object->intrinsic_trans = dmnsn_triangle_basis(vertices);
  return object;
}

dmnsn_object *
dmnsn_new_smooth_triangle(dmnsn_pool *pool, dmnsn_vector vertices[3], dmnsn_vector normals[3])
{
  dmnsn_matrix P = dmnsn_triangle_basis(vertices);

  /* Transform the given normals. */
  dmnsn_vector na = dmnsn_vector_normalized(dmnsn_transform_normal(P, normals[0]));
  dmnsn_vector nb = dmnsn_vector_normalized(dmnsn_transform_normal(P, normals[1]));
  dmnsn_vector nc = dmnsn_vector_normalized(dmnsn_transform_normal(P, normals[2]));

  dmnsn_smooth_triangle *triangle = DMNSN_PALLOC(pool, dmnsn_smooth_triangle);
  triangle->na  = na;
  triangle->nab = dmnsn_vector_sub(nb, na);
  triangle->nac = dmnsn_vector_sub(nc, na);

  dmnsn_object *object = &triangle->object;
  dmnsn_init_object(pool, object);
  object->intersection_fn = dmnsn_smooth_triangle_intersection_fn;
  object->inside_fn = dmnsn_triangle_inside_fn;
  object->bounding_box.min = dmnsn_zero;
  object->bounding_box.max = dmnsn_new_vector(1.0, 1.0, 0.0);
  object->intrinsic_trans = P;

  return object;
}
