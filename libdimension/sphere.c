/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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
#include <stdlib.h> /* For malloc */
#include <math.h>   /* For sqrt   */

static dmnsn_array *dmnsn_sphere_intersections_fn(dmnsn_line line);
static int dmnsn_sphere_inside_fn(dmnsn_vector point);

dmnsn_object *
dmnsn_new_sphere()
{
  dmnsn_object *sphere = dmnsn_new_object();
  sphere->intersections_fn = &dmnsn_sphere_intersections_fn;
  sphere->inside_fn        = &dmnsn_sphere_inside_fn;
  return sphere;
}

void
dmnsn_delete_sphere(dmnsn_object *sphere)
{
  dmnsn_delete_object(sphere);
}

static dmnsn_array *
dmnsn_sphere_intersections_fn(dmnsn_line line)
{
  double a, b, c, t[2];
  dmnsn_array *array = dmnsn_new_array(sizeof(double));

  // Solve (x0 + nx*t)^2 + (y0 + ny*t)^2 + (z0 + nz*t)^2 == 1

  a = line.n.x*line.n.x + line.n.y*line.n.y + line.n.z*line.n.z;
  b = 2.0*(line.n.x*line.x0.x + line.n.y*line.x0.y + line.n.z*line.x0.z);
  c = line.x0.x*line.x0.x + line.x0.y*line.x0.y + line.x0.z*line.x0.z - 1.0;

  if (b*b - 4.0*a*c >= 0) {
    t[0] = (-b + sqrt(b*b - 4.0*a*c))/(2*a);
    t[1] = (-b - sqrt(b*b - 4.0*a*c))/(2*a);
    dmnsn_array_set(array, 0, &t[0]);
    dmnsn_array_set(array, 1, &t[1]);
  }

  return array;
}

static int
dmnsn_sphere_inside_fn(dmnsn_vector point)
{
  return sqrt(point.x*point.x + point.y*point.y + point.z*point.z) < 1.0;
}
