/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Cameras.
 */

#include "dimension-internal.h"
#include <stdlib.h>

/* Allocate a new dummy camera */
dmnsn_camera *
dmnsn_new_camera(dmnsn_pool *pool)
{
  dmnsn_camera *camera = DMNSN_PALLOC(pool, dmnsn_camera);
  dmnsn_init_camera(camera);
  return camera;
}

/* Initialize a camera */
void
dmnsn_init_camera(dmnsn_camera *camera)
{
  camera->trans = dmnsn_identity_matrix();
}

/* Invoke the camera ray function */
dmnsn_line
dmnsn_camera_ray(const dmnsn_camera *camera, double x, double y)
{
  dmnsn_line ray = camera->ray_fn(camera, x, y);
  return dmnsn_transform_line(camera->trans, ray);
}
