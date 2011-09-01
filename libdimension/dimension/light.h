/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Lights.
 */

#include <stdbool.h>

/* Forward-declar dmnsn_light */
typedef struct dmnsn_light dmnsn_light;

/**
 * Light direction callback.
 * @param[in] light  The light itself.
 * @param[in] v      The point to illuminate.
 * @return The direction of light rays pointing from \p v
 */
typedef dmnsn_vector dmnsn_light_direction_fn(const dmnsn_light *light,
                                              dmnsn_vector v);

/**
 * Light illumination callback.
 * @param[in] light  The light itself.
 * @param[in] v      The point to illuminate.
 * @return The color of the light at \p v.
 */
typedef dmnsn_color dmnsn_light_illumination_fn(const dmnsn_light *light,
                                                dmnsn_vector v);

/**
 * Light shadow callback.
 * @param[in] light  The light itself.
 * @param[in] t      The line index of the closest shadow ray intersection.
 * @return Whether the point is in shadow.
 */
typedef bool dmnsn_light_shadow_fn(const dmnsn_light *light, double t);

/** A light. */
struct dmnsn_light {
  /* Callbacks */
  dmnsn_light_direction_fn    *direction_fn;    /**< Direction callback. */
  dmnsn_light_illumination_fn *illumination_fn; /**< Illumination callback. */
  dmnsn_light_shadow_fn       *shadow_fn;       /**< Shadow callback. */
  dmnsn_free_fn               *free_fn;         /**< Desctructor callback. */

  /** Generic pointer for light info. */
  void *ptr;

  /** @internal Reference count. */
  dmnsn_refcount refcount;
};

/**
 * Create a dummy light.
 * @return The allocated light.
 */
dmnsn_light *dmnsn_new_light(void);

/**
 * Delete a light.
 * @param[in,out] light  The light to delete.
 */
void dmnsn_delete_light(dmnsn_light *light);
