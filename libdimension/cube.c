/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
#include <math.h>  /* For sqrt */

/*
 * Cube
 */

/* Cube callbacks */
static bool dmnsn_cube_intersection_fn(const dmnsn_object *cube,
                                       dmnsn_line line,
                                       dmnsn_intersection *intersection);
static bool dmnsn_cube_inside_fn(const dmnsn_object *cube,
                                 dmnsn_vector point);

/* Allocate a new cube object */
dmnsn_object *
dmnsn_new_cube()
{
  dmnsn_object *cube = dmnsn_new_object();
  cube->intersection_fn  = &dmnsn_cube_intersection_fn;
  cube->inside_fn        = &dmnsn_cube_inside_fn;
  cube->bounding_box.min = dmnsn_new_vector(-1.0, -1.0, -1.0);
  cube->bounding_box.max = dmnsn_new_vector(1.0, 1.0, 1.0);
  return cube;
}

/* Intersections callback for a cube */
static bool
dmnsn_cube_intersection_fn(const dmnsn_object *cube, dmnsn_line line,
                           dmnsn_intersection *intersection)
{
  dmnsn_line line_trans = dmnsn_matrix_line_mul(cube->trans_inv, line);

  double t = -1.0, t_temp;
  dmnsn_vector p, normal;

  /* Six ray-plane intersection tests (x, y, z) = +/- 1.0 */

  if (line_trans.n.x != 0.0) {
    /* x = -1.0 */
    t_temp = (-1.0 - line_trans.x0.x)/line_trans.n.x;
    p = dmnsn_line_point(line_trans, t_temp);
    if (p.y >= -1.0 && p.y <= 1.0 && p.z >= -1.0 && p.z <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
      normal = dmnsn_new_vector(-1.0, 0.0, 0.0);
    }

    /* x = 1.0 */
    t_temp = (1.0 - line_trans.x0.x)/line_trans.n.x;
    p = dmnsn_line_point(line_trans, t_temp);
    if (p.y >= -1.0 && p.y <= 1.0 && p.z >= -1.0 && p.z <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
      normal = dmnsn_new_vector(1.0, 0.0, 0.0);
    }
  }

  if (line_trans.n.y != 0.0) {
    /* y = -1.0 */
    t_temp = (-1.0 - line_trans.x0.y)/line_trans.n.y;
    p = dmnsn_line_point(line_trans, t_temp);
    if (p.x >= -1.0 && p.x <= 1.0 && p.z >= -1.0 && p.z <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
      normal = dmnsn_new_vector(0.0, -1.0, 0.0);
    }

    /* y = 1.0 */
    t_temp = (1.0 - line_trans.x0.y)/line_trans.n.y;
    p = dmnsn_line_point(line_trans, t_temp);
    if (p.x >= -1.0 && p.x <= 1.0 && p.z >= -1.0 && p.z <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
      normal = dmnsn_new_vector(0.0, 1.0, 0.0);
    }
  }

  if (line_trans.n.z != 0.0) {
    /* z = -1.0 */
    t_temp = (-1.0 - line_trans.x0.z)/line_trans.n.z;
    p = dmnsn_line_point(line_trans, t_temp);
    if (p.x >= -1.0 && p.x <= 1.0 && p.y >= -1.0 && p.y <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
      normal = dmnsn_new_vector(0.0, 0.0, -1.0);
    }

    /* z = 1.0 */
    t_temp = (1.0 - line_trans.x0.z)/line_trans.n.z;
    p = dmnsn_line_point(line_trans, t_temp);
    if (p.x >= -1.0 && p.x <= 1.0 && p.y >= -1.0 && p.y <= 1.0
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
    {
      t = t_temp;
      normal = dmnsn_new_vector(0.0, 0.0, 1.0);
    }
  }

  if (t >= 0.0) {
    intersection->ray      = line;
    intersection->t        = t;
    intersection->normal   = dmnsn_matrix_normal_mul(cube->trans, normal);
    intersection->texture  = cube->texture;
    intersection->interior = cube->interior;
    return true;
  } else {
    return false;
  }
}

/* Inside callback for a cube */
static bool
dmnsn_cube_inside_fn(const dmnsn_object *cube, dmnsn_vector point)
{
  point = dmnsn_matrix_vector_mul(cube->trans_inv, point);
  return point.x > -1.0 && point.x < 1.0
      && point.y > -1.0 && point.y < 1.0
      && point.z > -1.0 && point.z < 1.0;
}
