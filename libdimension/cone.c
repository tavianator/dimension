/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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

/** Cone payload type. */
typedef struct dmnsn_cone_payload {
  double r1, r2;
  bool open;
} dmnsn_cone_payload;

/** Intersection callback for a cone. */
static bool
dmnsn_cone_intersection_fn(const dmnsn_object *cone, dmnsn_line l,
                           dmnsn_intersection *intersection)
{
  const dmnsn_cone_payload *payload = cone->ptr;
  double r1 = payload->r1, r2 = payload->r2;

  /* Solve (x0 + nx*t)^2 + (z0 + nz*t)^2
           == (((r2 - r1)*(y0 + ny*t) + r1 + r2)/2)^2 */
  double poly[3], x[2];
  poly[2] = l.n.x*l.n.x + l.n.z*l.n.z - l.n.y*l.n.y*(r2 - r1)*(r2 - r1)/4.0;
  poly[1] = 2.0*(l.n.x*l.x0.x + l.n.z*l.x0.z)
            - l.n.y*(r2 - r1)*(l.x0.y*(r2 - r1) + r2 + r1)/2.0;
  poly[0] = l.x0.x*l.x0.x + l.x0.z*l.x0.z
            - (l.x0.y*(r2 - r1) + r2 + r1)*(l.x0.y*(r2 - r1) + r2 + r1)/4;

  size_t n = dmnsn_solve_polynomial(poly, 2, x);

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

    if (!payload->open && l.n.y) {
      /* Test for cap intersections */
      double tcap1 = (-1.0 - l.x0.y)/l.n.y;
      double tcap2 = (+1.0 - l.x0.y)/l.n.y;

      double tcap, r;
      dmnsn_vector norm;
      if (tcap1 < tcap2) {
        tcap = tcap1;
        r    = r1;
        norm = dmnsn_new_vector(0.0, -1.0, 0.0);
      } else {
        tcap = tcap2;
        r    = r2;
        norm = dmnsn_new_vector(0.0, 1.0, 0.0);
      }
      dmnsn_vector pcap = dmnsn_line_point(l, tcap);

      if (tcap < 0.0 || pcap.x*pcap.x + pcap.z*pcap.z >= r*r) {
        if (tcap2 <= tcap1) {
          tcap = tcap1;
          r    = r1;
          norm = dmnsn_new_vector(0.0, -1.0, 0.0);
        } else {
          tcap = tcap2;
          r    = r2;
          norm = dmnsn_new_vector(0.0, 1.0, 0.0);
        }
        pcap = dmnsn_line_point(l, tcap);
      }

      if (tcap >= 0.0
          && (tcap < t || p.y <= -1.0 || p.y >= 1.0)
          && pcap.x*pcap.x + pcap.z*pcap.z < r*r)
      {
        intersection->t      = tcap;
        intersection->normal = norm;
        return true;
      }
    }

    if (t >= 0.0 && p.y > -1.0 && p.y < 1.0) {
      dmnsn_vector norm = dmnsn_vector_normalized(
        dmnsn_new_vector(p.x, -(r2 - r1)*sqrt(p.x*p.x + p.z*p.z)/2.0, p.z)
      );
      intersection->t      = t;
      intersection->normal = norm;
      return true;
    }
  }

  return false;
}

/** Inside callback for a cone. */
static bool
dmnsn_cone_inside_fn(const dmnsn_object *cone, dmnsn_vector point)
{
  const dmnsn_cone_payload *payload = cone->ptr;
  double r1 = payload->r1, r2 = payload->r2;
  double r = (point.y*(r2 - r1) + r1 + r2)/2.0;
  return point.x*point.x + point.z*point.z < r*r
         && point.y > -1.0 && point.y < 1.0;
}

/* Allocate a new cone object */
dmnsn_object *
dmnsn_new_cone(double r1, double r2, bool open)
{
  dmnsn_object *cone = dmnsn_new_object();
  cone->intersection_fn  = dmnsn_cone_intersection_fn;
  cone->inside_fn        = dmnsn_cone_inside_fn;
  double rmax = dmnsn_max(r1, r2);
  cone->bounding_box.min = dmnsn_new_vector(-rmax, -1.0, -rmax);
  cone->bounding_box.max = dmnsn_new_vector(rmax, 1.0, rmax);

  dmnsn_cone_payload *payload = dmnsn_malloc(sizeof(dmnsn_cone_payload));
  payload->r1       = r1;
  payload->r2       = r2;
  payload->open     = open;
  cone->ptr     = payload;
  cone->free_fn = dmnsn_free;
  return cone;
}
