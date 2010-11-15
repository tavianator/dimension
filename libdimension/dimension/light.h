/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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

#ifndef DIMENSION_LIGHT_H
#define DIMENSION_LIGHT_H

/* Forward-declar dmnsn_light */
typedef struct dmnsn_light dmnsn_light;

/**
 * Light callback.
 * @param[in] light  The light itself.
 * @param[in] v      The point to illuminate.
 * @return The color of the light at \p v.
 */
typedef dmnsn_color dmnsn_light_fn(const dmnsn_light *light, dmnsn_vector v);

/** A light. */
struct dmnsn_light {
  dmnsn_vector x0; /**< Origin of light rays */

  /* Callbacks */
  dmnsn_light_fn *light_fn; /**< Light callback. */
  dmnsn_free_fn  *free_fn;  /**< Desctructor callback. */

  /** Generic pointer for light info */
  void *ptr;
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

#endif /* DIMENSION_LIGHT_H */
