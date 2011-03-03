/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@tavianator.com>          *
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
 * Perspective cameras.
 */

#include "dimension.h"
#include <stdlib.h>

/** Perspective camera ray callback. */
static dmnsn_line
dmnsn_perspective_camera_ray_fn(const dmnsn_camera *camera, double x, double y)
{
  dmnsn_line l = dmnsn_new_line(
    dmnsn_zero,
    dmnsn_new_vector(x - 0.5, y - 0.5, 1.0)
  );
  return l;
}

/* Create a new perspective camera. */
dmnsn_camera *
dmnsn_new_perspective_camera(void)
{
  dmnsn_camera *camera = dmnsn_new_camera();
  camera->ray_fn  = &dmnsn_perspective_camera_ray_fn;
  return camera;
}
