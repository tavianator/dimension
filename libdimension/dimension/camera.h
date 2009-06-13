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

#ifndef DIMENSION_CAMERA_H
#define DIMENSION_CAMERA_H

/*
 * A camera.
 */

/* Forward-declare dmnsn_camera */
typedef struct dmnsn_camera dmnsn_camera;

/* Camera callback types */
typedef dmnsn_line dmnsn_camera_ray_fn(const dmnsn_camera *camera,
                                       const dmnsn_canvas *canvas,
                                       unsigned int x, unsigned int y);

struct dmnsn_camera {
  /* Generic pointer for camera info */
  void *ptr;

  /* Callback function */
  dmnsn_camera_ray_fn *ray_fn;
};

dmnsn_camera *dmnsn_new_camera();
void dmnsn_delete_camera(dmnsn_camera *camera);

/* A perspective camera, at the origin, looking at (0, 0, 1).  Rays are
   transformed by the transformation matrix `trans'. */

dmnsn_camera *dmnsn_new_perspective_camera(dmnsn_matrix trans);
void dmnsn_delete_perspective_camera(dmnsn_camera *camera);

#endif /* DIMENSION_CAMERA_H */
