/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "internal/polynomial.h"
#include "dimension/model.h"

/// Torus type.
typedef struct {
  dmnsn_object object;
  double major, minor;
} dmnsn_torus;

/// Bound the torus in a cylindrical shell.
static inline bool
dmnsn_torus_bound_intersection(const dmnsn_torus *torus, dmnsn_ray l)
{
  double R = torus->major, r = torus->minor;
  double rmax = R + r, rmin = R - r;
  double rmax2 = rmax*rmax, rmin2 = rmin*rmin;

  // Try the caps first
  double tlower = (-r - l.x0.Y)/l.n.Y;
  double tupper = (+r - l.x0.Y)/l.n.Y;
  dmnsn_vector lower = dmnsn_ray_point(l, tlower);
  dmnsn_vector upper = dmnsn_ray_point(l, tupper);
  double ldist2 = lower.X*lower.X + lower.Z*lower.Z;
  double udist2 = upper.X*upper.X + upper.Z*upper.Z;
  if ((ldist2 < rmin2 || ldist2 > rmax2) && (udist2 < rmin2 || udist2 > rmax2)) {
    // No valid intersection with the caps, try the cylinder walls
    double dist2 = l.x0.X*l.x0.X + l.x0.Z*l.x0.Z;
    double bigcyl[3], smallcyl[3];
    bigcyl[2]   = smallcyl[2] = l.n.X*l.n.X + l.n.Z*l.n.Z;
    bigcyl[1]   = smallcyl[1] = 2.0*(l.n.X*l.x0.X + l.n.Z*l.x0.Z);
    bigcyl[0]   = dist2 - rmax2;
    smallcyl[0] = dist2 - rmin2;

    double x[4];
    size_t n = dmnsn_polynomial_solve(bigcyl, 2, x);
    n += dmnsn_polynomial_solve(smallcyl, 2, x + n);

    size_t i;
    for (i = 0; i < n; ++i) {
      dmnsn_vector p = dmnsn_ray_point(l, x[i]);
      if (p.Y >= -r && p.Y <= r)
        break;
    }

    if (i == n) {
      // No valid intersection found
      return false;
    }
  }

  return true;
}

/// Torus intersection callback.
static bool
dmnsn_torus_intersection_fn(const dmnsn_object *object, dmnsn_ray l,
                            dmnsn_intersection *intersection)
{
  const dmnsn_torus *torus = (const dmnsn_torus *)object;
  double R = torus->major, r = torus->minor;
  double RR = R*R, rr = r*r;

  if (!dmnsn_torus_bound_intersection(torus, l)) {
    return false;
  }

  // This bit of algebra here is correct
  dmnsn_vector x0mod = dmnsn_new_vector(l.x0.X, -l.x0.Y, l.x0.Z);
  dmnsn_vector nmod  = dmnsn_new_vector(l.n.X,  -l.n.Y,  l.n.Z);
  double nn      = dmnsn_vector_dot(l.n, l.n);
  double nx0     = dmnsn_vector_dot(l.n, l.x0);
  double x0x0    = dmnsn_vector_dot(l.x0, l.x0);
  double x0x0mod = dmnsn_vector_dot(l.x0, x0mod);
  double nx0mod  = dmnsn_vector_dot(l.n, x0mod);
  double nnmod   = dmnsn_vector_dot(l.n, nmod);

  double poly[5];
  poly[4] = nn*nn;
  poly[3] = 4*nn*nx0;
  poly[2] = 2.0*(nn*(x0x0 - rr) + 2.0*nx0*nx0 - RR*nnmod);
  poly[1] = 4.0*(nx0*(x0x0 - rr) - RR*nx0mod);
  poly[0] = x0x0*x0x0 + RR*(RR - 2.0*x0x0mod) - rr*(2.0*(RR + x0x0) - rr);

  double x[4];
  size_t n = dmnsn_polynomial_solve(poly, 4, x);
  if (n == 0)
    return false;

  double t = x[0];
  for (size_t i = 1; i < n; ++i) {
    t = dmnsn_min(t, x[i]);
  }

  if (t < 0.0) {
    return false;
  }

  dmnsn_vector p = dmnsn_ray_point(l, t);
  dmnsn_vector center = dmnsn_vector_mul(
    R,
    dmnsn_vector_normalized(dmnsn_new_vector(p.X, 0.0, p.Z))
  );
  dmnsn_vector normal = dmnsn_vector_sub(p, center);

  intersection->t      = t;
  intersection->normal = normal;
  return true;
}

/// Torus inside callback.
static bool
dmnsn_torus_inside_fn(const dmnsn_object *object, dmnsn_vector point)
{
  const dmnsn_torus *torus = (const dmnsn_torus *)object;
  double dmajor = torus->major - sqrt(point.X*point.X + point.Z*point.Z);
  return dmajor*dmajor + point.Y*point.Y < torus->minor*torus->minor;
}

/// Torus bounding callback.
static dmnsn_aabb
dmnsn_torus_bounding_fn(const dmnsn_object *object, dmnsn_matrix trans)
{
  const dmnsn_torus *torus = (const dmnsn_torus *)object;

  double extent = torus->major + torus->minor;
  dmnsn_aabb box = dmnsn_symmetric_aabb(dmnsn_new_vector(extent, torus->minor, extent));
  return dmnsn_transform_aabb(trans, box);
}

/// Torus vtable.
static const dmnsn_object_vtable dmnsn_torus_vtable = {
  .intersection_fn = dmnsn_torus_intersection_fn,
  .inside_fn = dmnsn_torus_inside_fn,
  .bounding_fn = dmnsn_torus_bounding_fn,
};

dmnsn_object *
dmnsn_new_torus(dmnsn_pool *pool, double major, double minor)
{
  dmnsn_torus *torus = DMNSN_PALLOC(pool, dmnsn_torus);
  torus->major = major;
  torus->minor = minor;

  dmnsn_object *object = &torus->object;
  dmnsn_init_object(object);
  object->vtable = &dmnsn_torus_vtable;
  return object;
}
