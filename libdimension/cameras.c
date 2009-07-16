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

/*
 * Perspective camera
 */

/* Perspective camera ray callback */
static dmnsn_line dmnsn_perspective_camera_ray_fn(const dmnsn_camera *camera,
                                                  double x, double y);

/* Create a new perspective camera.  Rays are aimed from the origin to a screen
   located on the z = 1 frame, from (-0.5, -0.5) to (0.5, 0.5).  Rays are then
   transformed by the matrix `trans'. */
dmnsn_camera *
dmnsn_new_perspective_camera(dmnsn_matrix trans)
{
  dmnsn_matrix *ptr;
  dmnsn_camera *camera = dmnsn_new_camera();
  if (camera) {
    /* Allocate room for the transformation matrix */
    ptr = malloc(sizeof(dmnsn_matrix));
    if (!ptr) {
      dmnsn_delete_camera(camera);
      return NULL;
    }
    *ptr = trans;
    camera->ptr = ptr;

    camera->ray_fn  = &dmnsn_perspective_camera_ray_fn;
    camera->free_fn = &free;
  }
  return camera;
}

/* Get the transformation matrix */
dmnsn_matrix
dmnsn_get_perspective_camera_trans(const dmnsn_camera *camera)
{
  dmnsn_matrix *trans = camera->ptr;
  return *trans;
}

/* Set the transformation matrix */
void
dmnsn_set_perspective_camera_trans(dmnsn_camera *camera, dmnsn_matrix T)
{
  dmnsn_matrix *trans = camera->ptr;
  *trans = T;
}

/* Perspective camera ray callback */
static dmnsn_line
dmnsn_perspective_camera_ray_fn(const dmnsn_camera *camera,
                                double x, double y)
{
  dmnsn_matrix *trans = camera->ptr;
  dmnsn_line l;

  /* Rays originate at the origin, oddly enough */
  l.x0 = dmnsn_vector_construct(0.0, 0.0, 0.0);

  /* Aim at the z = 1 plane */
  l.n = dmnsn_vector_construct(x - 0.5, y - 0.5, 1.0);

  return dmnsn_matrix_line_mul(*trans, l);
}
