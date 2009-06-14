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

static dmnsn_array *dmnsn_cube_intersections_fn(const dmnsn_object *cube,
                                                  dmnsn_line line);
static int dmnsn_cube_inside_fn(const dmnsn_object *cube,
                                  dmnsn_vector point);

dmnsn_object *
dmnsn_new_cube()
{
  dmnsn_object *cube = dmnsn_new_object();
  if (cube) {
    cube->intersections_fn = &dmnsn_cube_intersections_fn;
    cube->inside_fn        = &dmnsn_cube_inside_fn;
  }
  return cube;
}

void
dmnsn_delete_cube(dmnsn_object *cube)
{
  dmnsn_delete_object(cube);
}

static dmnsn_array *
dmnsn_cube_intersections_fn(const dmnsn_object *cube, dmnsn_line line)
{
  double t;
  dmnsn_vector p;
  dmnsn_array *array = dmnsn_new_array(sizeof(double));

  /* Six ray-plane intersection tests (x, y, z) = +/- 1.0 */

  if (line.n.x != 0.0) {
    /* x = -1.0 */
    t = (-1.0 - line.x0.x)/line.n.x;
    p = dmnsn_line_point(line, t);
    if (p.y >= -1.0 && p.y <= 1.0 && p.z >= -1.0 && p.z <= 1.0) {
      dmnsn_array_push(array, &t);
    }

    /* x = 1.0 */
    t = (1.0 - line.x0.x)/line.n.x;
    p = dmnsn_line_point(line, t);
    if (p.y >= -1.0 && p.y <= 1.0 && p.z >= -1.0 && p.z <= 1.0) {
      dmnsn_array_push(array, &t);
    }
  }

  if (line.n.y != 0.0) {
    /* y = -1.0 */
    t = (-1.0 - line.x0.y)/line.n.y;
    p = dmnsn_line_point(line, t);
    if (p.x >= -1.0 && p.x <= 1.0 && p.z >= -1.0 && p.z <= 1.0) {
      dmnsn_array_push(array, &t);
    }

    /* y = 1.0 */
    t = (1.0 - line.x0.y)/line.n.y;
    p = dmnsn_line_point(line, t);
    if (p.x >= -1.0 && p.x <= 1.0 && p.z >= -1.0 && p.z <= 1.0) {
      dmnsn_array_push(array, &t);
    }
  }

  if (line.n.z != 0.0) {
    /* z = -1.0 */
    t = (-1.0 - line.x0.z)/line.n.z;
    p = dmnsn_line_point(line, t);
    if (p.x >= -1.0 && p.x <= 1.0 && p.y >= -1.0 && p.y <= 1.0) {
      dmnsn_array_push(array, &t);
    }

    /* z = 1.0 */
    t = (1.0 - line.x0.z)/line.n.z;
    p = dmnsn_line_point(line, t);
    if (p.x >= -1.0 && p.x <= 1.0 && p.y >= -1.0 && p.y <= 1.0) {
      dmnsn_array_push(array, &t);
    }
  }

  return array;
}

static int
dmnsn_cube_inside_fn(const dmnsn_object *cube, dmnsn_vector point)
{
  return point.x > -1.0 && point.x < 1.0
      && point.y > -1.0 && point.y < 1.0
      && point.z > -1.0 && point.z < 1.0; 
}
