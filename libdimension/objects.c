/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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

/*
 * Sphere
 */

/* Sphere object callbacks */

static dmnsn_intersection *
dmnsn_sphere_intersection_fn(const dmnsn_object *sphere, dmnsn_line line);

static int dmnsn_sphere_inside_fn(const dmnsn_object *sphere,
                                  dmnsn_vector point);

/* Allocate a new sphere */
dmnsn_object *
dmnsn_new_sphere()
{
  dmnsn_object *sphere = dmnsn_new_object();
  if (sphere) {
    sphere->intersection_fn = &dmnsn_sphere_intersection_fn;
    sphere->inside_fn       = &dmnsn_sphere_inside_fn;
    sphere->min             = dmnsn_vector_construct(-1.0, -1.0, -1.0);
    sphere->max             = dmnsn_vector_construct(1.0, 1.0, 1.0);
  }
  return sphere;
}

/* Returns the closest intersection of `line' with `sphere' */
static dmnsn_intersection *
dmnsn_sphere_intersection_fn(const dmnsn_object *sphere, dmnsn_line line)
{
  double a, b, c, t;
  dmnsn_intersection *intersection = NULL;

  /* Solve (x0 + nx*t)^2 + (y0 + ny*t)^2 + (z0 + nz*t)^2 == 1 */

  a = line.n.x*line.n.x + line.n.y*line.n.y + line.n.z*line.n.z;
  b = 2.0*(line.n.x*line.x0.x + line.n.y*line.x0.y + line.n.z*line.x0.z);
  c = line.x0.x*line.x0.x + line.x0.y*line.x0.y + line.x0.z*line.x0.z - 1.0;

  if (b*b - 4.0*a*c >= 0) {
    t = (-b - sqrt(b*b - 4.0*a*c))/(2*a);
    if (t < 0.0) {
      t = (-b + sqrt(b*b - 4.0*a*c))/(2*a);
    }

    if (t >= 0.0) {
      intersection = dmnsn_new_intersection();
      intersection->ray     = line;
      intersection->t       = t;
      intersection->texture = sphere->texture;
    }
  }

  return intersection;
}

/* Return whether a point is inside a sphere (x**2 + y**2 + z**2 < 1.0) */
static int
dmnsn_sphere_inside_fn(const dmnsn_object *sphere, dmnsn_vector point)
{
  return point.x*point.x + point.y*point.y + point.z*point.z < 1.0;
}

/*
 * Cube
 */

/* Cube callbacks */
static dmnsn_intersection *dmnsn_cube_intersection_fn(const dmnsn_object *cube,
                                                      dmnsn_line line);
static int dmnsn_cube_inside_fn(const dmnsn_object *cube,
                                  dmnsn_vector point);

/* Allocate a new cube object */
dmnsn_object *
dmnsn_new_cube()
{
  dmnsn_object *cube = dmnsn_new_object();
  if (cube) {
    cube->intersection_fn = &dmnsn_cube_intersection_fn;
    cube->inside_fn       = &dmnsn_cube_inside_fn;
    cube->min             = dmnsn_vector_construct(-1.0, -1.0, -1.0);
    cube->max             = dmnsn_vector_construct(1.0, 1.0, 1.0);
  }
  return cube;
}

/* Intersections callback for a cube */
static dmnsn_intersection *
dmnsn_cube_intersection_fn(const dmnsn_object *cube, dmnsn_line line)
{
  double t = -1.0, t_temp;
  dmnsn_vector p;
  dmnsn_intersection *intersection = NULL;

  /* Six ray-plane intersection tests (x, y, z) = +/- 1.0 */

  if (line.n.x != 0.0) {
    /* x = -1.0 */
    t_temp = (-1.0 - line.x0.x)/line.n.x;
    p = dmnsn_line_point(line, t_temp);
    if (p.y >= -1.0 && p.y <= 1.0 && p.z >= -1.0 && p.z <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
    }

    /* x = 1.0 */
    t_temp = (1.0 - line.x0.x)/line.n.x;
    p = dmnsn_line_point(line, t_temp);
    if (p.y >= -1.0 && p.y <= 1.0 && p.z >= -1.0 && p.z <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
    }
  }

  if (line.n.y != 0.0) {
    /* y = -1.0 */
    t_temp = (-1.0 - line.x0.y)/line.n.y;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= -1.0 && p.x <= 1.0 && p.z >= -1.0 && p.z <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
    }

    /* y = 1.0 */
    t_temp = (1.0 - line.x0.y)/line.n.y;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= -1.0 && p.x <= 1.0 && p.z >= -1.0 && p.z <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
    }
  }

  if (line.n.z != 0.0) {
    /* z = -1.0 */
    t_temp = (-1.0 - line.x0.z)/line.n.z;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= -1.0 && p.x <= 1.0 && p.y >= -1.0 && p.y <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
    }

    /* z = 1.0 */
    t_temp = (1.0 - line.x0.z)/line.n.z;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= -1.0 && p.x <= 1.0 && p.y >= -1.0 && p.y <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
    }
  }

  if (t >= 0.0) {
    intersection = dmnsn_new_intersection();
    intersection->ray     = line;
    intersection->t       = t;
    intersection->texture = cube->texture;
  }

  return intersection;
}

/* Inside callback for a cube */
static int
dmnsn_cube_inside_fn(const dmnsn_object *cube, dmnsn_vector point)
{
  return point.x > -1.0 && point.x < 1.0
      && point.y > -1.0 && point.y < 1.0
      && point.z > -1.0 && point.z < 1.0;
}
