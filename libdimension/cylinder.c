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

#include "dimension.h"
#include <math.h>

/*
 * Cylinder
 */

/* Cylinder callbacks */
static bool dmnsn_cylinder_intersection_fn(const dmnsn_object *cylinder,
                                           dmnsn_line line,
                                           dmnsn_intersection *intersection);
static bool dmnsn_cylinder_inside_fn(const dmnsn_object *cylinder,
                                     dmnsn_vector point);

/* Allocate a new cylinder object */
dmnsn_object *
dmnsn_new_cylinder(bool open)
{
  dmnsn_object *cylinder = dmnsn_new_object();
  cylinder->intersection_fn  = &dmnsn_cylinder_intersection_fn;
  cylinder->inside_fn        = &dmnsn_cylinder_inside_fn;
  cylinder->bounding_box.min = dmnsn_new_vector(-1.0, -1.0, -1.0);
  cylinder->bounding_box.max = dmnsn_new_vector(1.0, 1.0, 1.0);
  if (open) {
    cylinder->ptr = cylinder; /* (bool)cyliner->ptr == open */
  }
  return cylinder;
}

/* Intersections callback for a cylinder */
static bool
dmnsn_cylinder_intersection_fn(const dmnsn_object *cylinder, dmnsn_line line,
                               dmnsn_intersection *intersection)
{
  dmnsn_line l = dmnsn_transform_line(cylinder->trans_inv, line);

  /* Solve (x0 + nx*t)^2 + (z0 + nz*t)^2 == 1 */
  double a, b, c, t;
  a = l.n.x*l.n.x + l.n.z*l.n.z;
  b = 2.0*(l.n.x*l.x0.x + l.n.z*l.x0.z);
  c = l.x0.x*l.x0.x + l.x0.z*l.x0.z - 1.0;

  if (b*b - 4.0*a*c >= 0.0) {
    t = (-b - sqrt(b*b - 4.0*a*c))/(2.0*a);
    dmnsn_vector p = dmnsn_line_point(l, t);

    if (t < 0.0 || p.y <= -1.0 || p.y >= 1.0) {
      t = (-b + sqrt(b*b - 4.0*a*c))/(2.0*a);
      p = dmnsn_line_point(l, t);
    }

    if (!cylinder->ptr) {
      /* Test for cap intersections */
      double tcap = (-1.0 - l.x0.y)/l.n.y;
      dmnsn_vector pcap = dmnsn_line_point(l, tcap);
      dmnsn_vector norm = dmnsn_new_vector(0.0, -1.0, 0.0);

      if (tcap < 0.0 || pcap.x*pcap.x + pcap.z*pcap.z >= 1.0) {
        tcap = (+1.0 - l.x0.y)/l.n.y;
        pcap = dmnsn_line_point(l, tcap);
        norm = dmnsn_new_vector(0.0, 1.0, 0.0);
      }

      if (tcap >= 0.0
          && (tcap < t || p.y <= -1.0 || p.y >= 1.0)
          && pcap.x*pcap.x + pcap.z*pcap.z < 1.0)
      {
        intersection->ray      = line;
        intersection->t        = tcap;
        intersection->normal   = dmnsn_transform_normal(cylinder->trans, norm);
        intersection->texture  = cylinder->texture;
        intersection->interior = cylinder->interior;
        return true;
      }
    }

    if (t >= 0.0 && p.y > -1.0 && p.y < 1.0) {
      p.y = 0;
      intersection->ray      = line;
      intersection->t        = t;
      intersection->normal   = dmnsn_transform_normal(cylinder->trans, p);
      intersection->texture  = cylinder->texture;
      intersection->interior = cylinder->interior;
      return true;
    }
  }

  return false;
}

/* Inside callback for a cylinder */
static bool
dmnsn_cylinder_inside_fn(const dmnsn_object *cylinder, dmnsn_vector point)
{
  point = dmnsn_transform_vector(cylinder->trans_inv, point);
  return point.x*point.x + point.z*point.z < 1
      && point.y > -1.0 && point.y < 1.0;
}
