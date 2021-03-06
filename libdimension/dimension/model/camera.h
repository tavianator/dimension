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

/* Forward-declare dmnsn_camera */
typedef struct dmnsn_camera dmnsn_camera;

/**
 * Camera ray callback.
 * @param[in] camera  The camera itself.
 * @param[in] x       The x coordinate of the pixel (in [0, 1]).
 * @param[in] y       The y coordinate of the pixel (in [0, 1]).
 * @return The ray through (\p x, \p y).
 */
typedef dmnsn_ray dmnsn_camera_ray_fn(const dmnsn_camera *camera, double x, double y);

/** A camera. */
struct dmnsn_camera {
  /* Callback functions */
  dmnsn_camera_ray_fn *ray_fn; /**< Camera ray callback. */

  dmnsn_matrix trans; /**< Transformation matrix. */
};

/**
 * Create a dummy camera.
 * @param[in] pool  The memory pool to allocate from.
 * @return The allocated camera.
 */
dmnsn_camera *dmnsn_new_camera(dmnsn_pool *pool);

/**
 * Initialize a dmnsn_camera field.
 * @param[out] camera  The camera to initialize.
 */
void dmnsn_init_camera(dmnsn_camera *camera);

/**
 * Invoke the camera ray callback, then correctly transform the ray.
 * @param[in] camera  The camera itself.
 * @param[in] x       The x coordinate of the pixel (in [0, 1]).
 * @param[in] y       The y coordinate of the pixel (in [0, 1]).
 * @return The ray through (\p x, \p y).
 */
dmnsn_ray dmnsn_camera_ray(const dmnsn_camera *camera, double x, double y);
