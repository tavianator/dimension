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
 * Sky spheres.
 */

/** A sky sphere. */
typedef struct dmnsn_sky_sphere {
  /** An array of pigments in inside-to-outside order. */
  dmnsn_array *pigments;

  dmnsn_matrix trans; /**< Transformation matrix. */

  dmnsn_refcount refcount; /**< @internal Reference count. */
} dmnsn_sky_sphere;

/**
 * Create a sky sphere.
 * @return A new blank sky sphere.
 */
dmnsn_sky_sphere *dmnsn_new_sky_sphere(void);

/**
 * Delete a sky sphere.
 * @param[in,out] sky_sphere  The sky sphere to delete.
 */
void dmnsn_delete_sky_sphere(dmnsn_sky_sphere *sky_sphere);

/**
 * Initialize a sky sphere.
 * @param[in,out] sky_sphere  The sky sphere to initialize.
 */
void dmnsn_initialize_sky_sphere(dmnsn_sky_sphere *sky_sphere);

/**
 * Evaluate the color of the sky sphere.
 * @param[in] sky_sphere  The sky sphere to evaluate.
 * @param[in] d           The direction to look.
 * @return The color of the sky in the direction of \p d.
 */
dmnsn_color dmnsn_sky_sphere_color(const dmnsn_sky_sphere *sky_sphere,
                                   dmnsn_vector d);
