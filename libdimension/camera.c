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

/* Allocate a new dummy camera */
dmnsn_camera *
dmnsn_new_camera()
{
  return malloc(sizeof(dmnsn_camera));
}

/* Free a dummy camera */
void
dmnsn_delete_camera(dmnsn_camera *camera)
{
  free(camera);
}

/* Perspective camera */

/* Perspective camera ray callback */
static dmnsn_line dmnsn_perspective_camera_ray_fn(const dmnsn_camera *camera,
                                                  const dmnsn_canvas *canvas,
                                                  unsigned int x,
                                                  unsigned int y);

/* Create a new perspective camera.  Rays are aimed from the origin to a screen
   located on the z = 1 frame, from (-0.5, -0.5) to (0.5, 0.5).  Rays are then
   transformed by the matrix `trans'. */
dmnsn_camera *
dmnsn_new_perspective_camera(dmnsn_matrix trans)
{
  dmnsn_camera *camera = dmnsn_new_camera();
  if (camera) {
    camera->ray_fn = &dmnsn_perspective_camera_ray_fn;

    camera->ptr = malloc(sizeof(dmnsn_matrix));
    if (!camera->ptr) {
      dmnsn_delete_camera(camera);
      return NULL;
    }
    *((dmnsn_matrix*)camera->ptr) = trans;
  }
  return camera;
}

/* Delete a perspective camera */
void
dmnsn_delete_perspective_camera(dmnsn_camera *camera)
{
  if (camera) {
    free(camera->ptr);
    dmnsn_delete_camera(camera);
  }
}

static dmnsn_line
dmnsn_perspective_camera_ray_fn(const dmnsn_camera *camera,
                                const dmnsn_canvas *canvas,
                                unsigned int x, unsigned int y)
{
  dmnsn_matrix *trans = (dmnsn_matrix *)camera->ptr;
  dmnsn_line l;

  /* Rays originate at the origin, oddly enough */
  l.x0 = dmnsn_vector_construct(0.0, 0.0, 0.0);

  /* Aim at the z = 1 plane */
  l.n.x = ((double)x)/(canvas->x - 1) - 0.5;
  l.n.y = ((double)y)/(canvas->y - 1) - 0.5;
  l.n.z = 1.0;

  return dmnsn_matrix_line_mul(*trans, l);
}
