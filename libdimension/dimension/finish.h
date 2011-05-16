/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Object finishes.
 */

/* Forward-declare dmnsn_finish */
typedef struct dmnsn_finish dmnsn_finish;

/**
 * Diffuse reflection callback.
 * @param[in] finish  The finish itself.
 * @param[in] light   The color of the light illuminating the object.
 * @param[in] color   The pigment of the object.
 * @param[in] ray     The direction of the light source.
 * @param[in] normal  The normal vector of the surface.
 * @return The diffuse reflection component of the object's color.
 */
typedef dmnsn_color dmnsn_diffuse_fn(const dmnsn_finish *finish,
                                     dmnsn_color light, dmnsn_color color,
                                     dmnsn_vector ray, dmnsn_vector normal);
/**
 * Specular highlight callback.
 * @param[in] finish  The finish itself.
 * @param[in] light   The color of the light illuminating the object.
 * @param[in] color   The pigment of the object.
 * @param[in] ray     The direction of the light source.
 * @param[in] normal  The normal vector of the surface.
 * @param[in] viewer  The direction of the viewer.
 * @return The specular reflection component of the object's color.
 */
typedef dmnsn_color dmnsn_specular_fn(const dmnsn_finish *finish,
                                      dmnsn_color light, dmnsn_color color,
                                      dmnsn_vector ray, dmnsn_vector normal,
                                      dmnsn_vector viewer);
/**
 * Ambient light callback.
 * @param[in] finish   The finish itself.
 * @param[in] pigment  The pigment of the object.
 * @return The ambient contribution to the object's color.
 */
typedef dmnsn_color dmnsn_ambient_fn(const dmnsn_finish *finish,
                                     dmnsn_color pigment);
/**
 * Reflected light callback.
 * @param[in] finish   The finish itself.
 * @param[in] reflect  The color of the reflected ray.
 * @param[in] color    The pigment of the object.
 * @param[in] ray      The direction of the reflected ray.
 * @param[in] normal   The normal vector of the surface.
 * @return The contribution of the reflected ray to the object's color.
 */
typedef dmnsn_color dmnsn_reflection_fn(const dmnsn_finish *finish,
                                        dmnsn_color reflect, dmnsn_color color,
                                        dmnsn_vector ray, dmnsn_vector normal);

/** A finish. */
struct dmnsn_finish {
  dmnsn_diffuse_fn    *diffuse_fn;    /**< The diffuse callback. */
  dmnsn_specular_fn   *specular_fn;   /**< The specular callback. */
  dmnsn_ambient_fn    *ambient_fn;    /**< The ambient callback. */
  dmnsn_reflection_fn *reflection_fn; /**< The reflection callback. */
  dmnsn_free_fn       *free_fn;       /**< The destruction callback. */

  /** Generic pointer. */
  void *ptr;
};

/**
 * Allocate a new dummy finish.
 * @return The allocated finish.
 */
dmnsn_finish *dmnsn_new_finish(void);

/**
 * Delete a finish.
 * @param[in,out] finish  The finish to delete.
 */
void dmnsn_delete_finish(dmnsn_finish *finish);
