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
 * Cones/cylinders.
 */

#include "dimension.h"
#include <math.h>

/** Cone type. */
typedef struct dmnsn_cone {
  dmnsn_object object;
  double r1, r2;
} dmnsn_cone;

/** Intersection callback for a cone. */
static bool
dmnsn_cone_intersection_fn(const dmnsn_object *object, dmnsn_line l,
                           dmnsn_intersection *intersection)
{
  const dmnsn_cone *cone = (const dmnsn_cone *)object;
  double r1 = cone->r1, r2 = cone->r2;

  /* Solve (x0 + nx*t)^2 + (z0 + nz*t)^2
           == (((r2 - r1)*(y0 + ny*t) + r1 + r2)/2)^2 */
  double poly[3], x[2];
  poly[2] = l.n.x*l.n.x + l.n.z*l.n.z - l.n.y*l.n.y*(r2 - r1)*(r2 - r1)/4.0;
  poly[1] = 2.0*(l.n.x*l.x0.x + l.n.z*l.x0.z)
            - l.n.y*(r2 - r1)*(l.x0.y*(r2 - r1) + r2 + r1)/2.0;
  poly[0] = l.x0.x*l.x0.x + l.x0.z*l.x0.z
            - (l.x0.y*(r2 - r1) + r2 + r1)*(l.x0.y*(r2 - r1) + r2 + r1)/4.0;

  size_t n = dmnsn_polynomial_solve(poly, 2, x);

  if (n > 0) {
    double t = x[0];
    dmnsn_vector p;
    if (n == 2) {
      t = dmnsn_min(t, x[1]);
      p = dmnsn_line_point(l, t);

      if (p.y <= -1.0 || p.y >= 1.0) {
        t = dmnsn_max(x[0], x[1]);
        p = dmnsn_line_point(l, t);
      }
    } else {
      p = dmnsn_line_point(l, t);
    }

    if (t >= 0.0 && p.y >= -1.0 && p.y <= 1.0) {
      double r = ((r2 - r1)*p.y + r1 + r2)/2.0;
      dmnsn_vector norm = dmnsn_new_vector(p.x, -r*(r2 - r1)/2.0, p.z);
      intersection->t      = t;
      intersection->normal = norm;
      return true;
    }
  }

  return false;
}

/** Inside callback for a cone. */
static bool
dmnsn_cone_inside_fn(const dmnsn_object *object, dmnsn_vector point)
{
  const dmnsn_cone *cone = (const dmnsn_cone *)cone;
  double r1 = cone->r1, r2 = cone->r2;
  double r = (point.y*(r2 - r1) + r1 + r2)/2.0;
  return point.x*point.x + point.z*point.z < r*r
         && point.y > -1.0 && point.y < 1.0;
}

/** Cone cap type. */
typedef struct dmnsn_cone_cap {
  dmnsn_object object;
  double r;
} dmnsn_cone_cap;

/** Cone cap intersection function. */
static bool
dmnsn_cone_cap_intersection_fn(const dmnsn_object *object, dmnsn_line l,
                               dmnsn_intersection *intersection)
{
  if (l.n.y != 0.0) {
    const dmnsn_cone_cap *cap = (const dmnsn_cone_cap *)object;
    double r = cap->r;
    double t = -l.x0.y/l.n.y;
    dmnsn_vector p = dmnsn_line_point(l, t);
    if (t >= 0.0 && p.x*p.x + p.z*p.z <= r*r) {
      intersection->t      = t;
      intersection->normal = dmnsn_new_vector(0.0, -1.0, 0.0);
      return true;
    }
  }

  return false;
}

/** Inside callback for a cone cap. */
static bool
dmnsn_cone_cap_inside_fn(const dmnsn_object *object, dmnsn_vector point)
{
  return false;
}

/** Allocate a new cone cap. */
dmnsn_object *
dmnsn_new_cone_cap(double r)
{
  dmnsn_cone_cap *cap = DMNSN_MALLOC(dmnsn_cone_cap);
  dmnsn_init_object(&cap->object);
  cap->object.intersection_fn  = dmnsn_cone_cap_intersection_fn;
  cap->object.inside_fn = dmnsn_cone_cap_inside_fn;
  cap->object.bounding_box.min = dmnsn_new_vector(-r, 0.0, -r);
  cap->object.bounding_box.max = dmnsn_new_vector(+r, 0.0, +r);
  cap->r = r;
  return &cap->object;
}

/* Allocate a new cone object */
dmnsn_object *
dmnsn_new_cone(double r1, double r2, bool open)
{
  dmnsn_cone *cone = DMNSN_MALLOC(dmnsn_cone);
  dmnsn_init_object(&cone->object);
  cone->object.intersection_fn  = dmnsn_cone_intersection_fn;
  cone->object.inside_fn = dmnsn_cone_inside_fn;
  double rmax = dmnsn_max(r1, r2);
  cone->object.bounding_box.min = dmnsn_new_vector(-rmax, -1.0, -rmax);
  cone->object.bounding_box.max = dmnsn_new_vector(rmax, 1.0, rmax);
  cone->r1 = r1;
  cone->r2 = r2;

  if (open) {
    return &cone->object;
  }

  /* Implement closed cones as a union with the caps */
  dmnsn_object *cap1 = dmnsn_new_cone_cap(r1);
  dmnsn_object *cap2 = dmnsn_new_cone_cap(r2);
  cap1->intrinsic_trans  = dmnsn_translation_matrix(
    dmnsn_new_vector(0.0, -1.0, 0.0)
  );
  cap2->intrinsic_trans  = dmnsn_translation_matrix(
    dmnsn_new_vector(0.0, +1.0, 0.0)
  );
  /* Flip the normal around for the top cap */
  cap2->intrinsic_trans.n[1][1] = -1.0;

  dmnsn_array *withcaps = dmnsn_new_array(sizeof(dmnsn_object *));
  dmnsn_array_push(withcaps, &cone);
  dmnsn_array_push(withcaps, &cap1);
  dmnsn_array_push(withcaps, &cap2);
  dmnsn_object *cone_cap_union = dmnsn_new_csg_union(withcaps);
  dmnsn_delete_array(withcaps);

  return cone_cap_union;
}
