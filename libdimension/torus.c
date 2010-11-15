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

/**
 * @file
 * Torii.  A special case of a quartic.
 */

#include "dimension.h"

/** Torus payload type. */
typedef struct dmnsn_torus_payload {
  double major, minor;
} dmnsn_torus_payload;

/** Bound the torus in a cylindrical shell. */
static inline bool
dmnsn_torus_bound_intersection(const dmnsn_torus_payload *payload, dmnsn_line l)
{
  double R = payload->major, r = payload->minor;
  double rmax = R + r, rmin = R - r;
  double rmax2 = rmax*rmax, rmin2 = rmin*rmin;

  /* Try the caps first */
  double tlower = (-r - l.x0.y)/l.n.y;
  double tupper = (+r - l.x0.y)/l.n.y;
  dmnsn_vector lower = dmnsn_line_point(l, tlower);
  dmnsn_vector upper = dmnsn_line_point(l, tupper);
  double ldist2 = lower.x*lower.x + lower.z*lower.z;
  double udist2 = upper.x*upper.x + upper.z*upper.z;
  if ((ldist2 < rmin2 || ldist2 > rmax2) && (udist2 < rmin2 || udist2 > rmax2))
  {
    /* No valid intersection with the caps, try the cylinder walls */
    double dist2 = l.x0.x*l.x0.x + l.x0.z*l.x0.z;
    double bigcyl[3], smallcyl[3];
    bigcyl[2]   = smallcyl[2] = l.n.x*l.n.x + l.n.z*l.n.z;
    bigcyl[1]   = smallcyl[1] = 2.0*(l.n.x*l.x0.x + l.n.z*l.x0.z);
    bigcyl[0]   = dist2 - rmax2;
    smallcyl[0] = dist2 - rmin2;

    double x[4];
    size_t n = dmnsn_solve_polynomial(bigcyl, 2, x);
    n += dmnsn_solve_polynomial(smallcyl, 2, x + n);

    size_t i;
    for (i = 0; i < n; ++i) {
      dmnsn_vector p = dmnsn_line_point(l, x[i]);
      if (p.y >= -r && p.y <= r)
        break;
    }

    if (i == n) {
      /* No valid intersection found */
      return false;
    }
  }

  return true;
}

/** Torus intersection callback. */
static bool
dmnsn_torus_intersection_fn(const dmnsn_object *torus, dmnsn_line l,
                            dmnsn_intersection *intersection)
{
  const dmnsn_torus_payload *payload = torus->ptr;
  double R = payload->major, r = payload->minor;
  double R2 = R*R, r2 = r*r;

  if (!dmnsn_torus_bound_intersection(payload, l))
    return false;

  /* This bit of algebra here is correct */
  dmnsn_vector x0mod = dmnsn_new_vector(l.x0.x, -l.x0.y, l.x0.z);
  dmnsn_vector nmod  = dmnsn_new_vector(l.n.x,  -l.n.y,  l.n.z);
  double nn      = dmnsn_vector_dot(l.n, l.n);
  double nx0     = dmnsn_vector_dot(l.n, l.x0);
  double x0x0    = dmnsn_vector_dot(l.x0, l.x0);
  double x0x0mod = dmnsn_vector_dot(l.x0, x0mod);
  double nx0mod  = dmnsn_vector_dot(l.n, x0mod);
  double nnmod   = dmnsn_vector_dot(l.n, nmod);

  double poly[5];
  poly[4] = nn*nn;
  poly[3] = 4*nn*nx0;
  poly[2] = 2.0*(nn*(x0x0 - r2) + 2.0*nx0*nx0 - R2*nnmod);
  poly[1] = 4.0*(nx0*(x0x0 - r2) - R2*nx0mod);
  poly[0] = x0x0*x0x0 + R2*(R2 - 2.0*x0x0mod) - r2*(2.0*(R2 + x0x0) - r2);

  double x[4];
  size_t n = dmnsn_solve_polynomial(poly, 4, x);
  if (n == 0)
    return false;

  double t = x[0];
  for (size_t i = 1; i < n; ++i) {
    t = dmnsn_min(t, x[i]);
  }

  if (t < 0.0)
    return false;

  dmnsn_vector p = dmnsn_line_point(l, t);
  dmnsn_vector center = dmnsn_vector_mul(
    payload->major,
    dmnsn_vector_normalize(dmnsn_new_vector(p.x, 0.0, p.z))
  );
  dmnsn_vector normal = dmnsn_vector_normalize(dmnsn_vector_sub(p, center));
  intersection->ray      = l;
  intersection->t        = t;
  intersection->normal   = normal;
  intersection->texture  = torus->texture;
  intersection->interior = torus->interior;
  return true;
}

/** Torus inside callback. */
static bool
dmnsn_torus_inside_fn(const dmnsn_object *torus, dmnsn_vector point)
{
  const dmnsn_torus_payload *payload = torus->ptr;
  double dmajor = payload->major - sqrt(point.x*point.x + point.z*point.z);
  return dmajor*dmajor + point.y*point.y < payload->minor*payload->minor;
}

/* Allocate a new torus */
dmnsn_object *
dmnsn_new_torus(double major, double minor)
{
  dmnsn_object *torus = dmnsn_new_object();
  torus->intersection_fn  = &dmnsn_torus_intersection_fn;
  torus->inside_fn        = &dmnsn_torus_inside_fn;
  torus->bounding_box.min = dmnsn_new_vector(-(major + minor),
                                             -minor,
                                             -(major + minor));
  torus->bounding_box.max = dmnsn_new_vector(major + minor,
                                             minor,
                                             major + minor);

  dmnsn_torus_payload *payload = dmnsn_malloc(sizeof(dmnsn_torus_payload));
  payload->major = major;
  payload->minor = minor;
  torus->ptr     = payload;
  torus->free_fn = &dmnsn_free;
  return torus;
}
