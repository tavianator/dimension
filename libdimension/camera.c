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

dmnsn_camera *
dmnsn_new_camera()
{
  return malloc(sizeof(dmnsn_camera));
}

void
dmnsn_delete_camera(dmnsn_camera *camera)
{
  free(camera);
}

/* Perspective camera */

static dmnsn_line dmnsn_perspective_camera_ray_fn(const dmnsn_canvas *canvas,
                                                  unsigned int x,
                                                  unsigned int y);

dmnsn_camera *
dmnsn_new_perspective_camera()
{
  dmnsn_camera *camera = dmnsn_new_camera();
  camera->ray_fn = &dmnsn_perspective_camera_ray_fn;
  return camera;
}

void
dmnsn_delete_perspective_camera(dmnsn_camera *camera)
{
  dmnsn_delete_camera(camera);
}

static dmnsn_line
dmnsn_perspective_camera_ray_fn(const dmnsn_canvas *canvas,
                                unsigned int x, unsigned int y)
{
  dmnsn_line l;
  l.x0 = dmnsn_vector_construct(0.0, 0.0, 0.0);
  l.n.x = ((double)x)/(canvas->x - 1) - 0.5;
  l.n.y = ((double)y)/(canvas->y - 1) - 0.5;
  l.n.z = 1.0;
  return l;
}
