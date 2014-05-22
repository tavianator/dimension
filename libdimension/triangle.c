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
 * Triangles.
 */

#include "dimension.h"

typedef struct dmnsn_triangle_payload {
  dmnsn_vector na, nab, nac;
} dmnsn_triangle_payload;

/** Triangle intersection callback. */
static bool
dmnsn_triangle_intersection_fn(const dmnsn_object *triangle, dmnsn_line l,
                               dmnsn_intersection *intersection)
{
  const dmnsn_triangle_payload *payload = triangle->ptr;

  /* See the change of basis in dmnsn_new_triangle() */
  double t = -l.x0.z/l.n.z;
  double u = l.x0.x + t*l.n.x;
  double v = l.x0.y + t*l.n.y;
  if (t >= 0.0 && u >= 0.0 && v >= 0.0 && u + v <= 1.0) {
    intersection->t      = t;
    intersection->normal = dmnsn_vector_add(
      payload->na,
      dmnsn_vector_add(
        dmnsn_vector_mul(u, payload->nab),
        dmnsn_vector_mul(v, payload->nac)
      )
    );
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
dmnsn_new_triangle(dmnsn_vector a, dmnsn_vector b, dmnsn_vector c,
                   dmnsn_vector na, dmnsn_vector nb, dmnsn_vector nc)
{
  na = dmnsn_vector_normalized(na);
  nb = dmnsn_vector_normalized(nb);
  nc = dmnsn_vector_normalized(nc);

  dmnsn_triangle_payload *payload = DMNSN_MALLOC(dmnsn_triangle_payload);
  payload->na  = na;
  payload->nab = dmnsn_vector_sub(nb, na);
  payload->nac = dmnsn_vector_sub(nc, na);

  dmnsn_object *triangle     = dmnsn_new_object();
  triangle->ptr              = payload;
  triangle->intersection_fn  = dmnsn_triangle_intersection_fn;
  triangle->inside_fn        = dmnsn_triangle_inside_fn;
  triangle->free_fn          = dmnsn_free;
  triangle->bounding_box.min = dmnsn_zero;
  triangle->bounding_box.max = dmnsn_new_vector(1.0, 1.0, 0.0);

  /*
   * Make a change-of-basis matrix
   *
   * The new vector space has corners at <0, 1, 0>, <0, 0, 1>, and 0,
   * corresponding to the basis (ab, ac, ab X ac).
   */
  dmnsn_vector ab = dmnsn_vector_sub(b, a);
  dmnsn_vector ac = dmnsn_vector_sub(c, a);
  dmnsn_vector normal = dmnsn_vector_cross(ab, ac);
  triangle->intrinsic_trans = dmnsn_new_matrix4(ab, ac, normal, a);

  return triangle;
}

/* Allocate a new flat triangle */
dmnsn_object *
dmnsn_new_flat_triangle(dmnsn_vector a, dmnsn_vector b, dmnsn_vector c)
{
  /* Flat triangles are just smooth triangles with identical normals at all
     verticies */
  dmnsn_vector normal = dmnsn_vector_cross(
    dmnsn_vector_sub(b, a),
    dmnsn_vector_sub(c, a)
  );
  return dmnsn_new_triangle(a, b, c, normal, normal, normal);
}
