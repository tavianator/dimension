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
 * Object textures.
 */

/** A complete texture. */
typedef struct {
  dmnsn_pigment *pigment; /**< Pigment. */
  dmnsn_finish finish;    /**< Finish. */

  dmnsn_matrix trans;     /**< Transformation matrix. */
  dmnsn_matrix trans_inv; /**< The inverse of the transformation matrix. */

  dmnsn_refcount refcount; /**< @internal Reference count. */
  bool initialized; /**< @internal Whether the texture is initialized yet. */
} dmnsn_texture;

/**
 * Create a blank texture.
 * @return The new texture.
 */
dmnsn_texture *dmnsn_new_texture(void);

/**
 * Delete a texture.
 * @param[in,out] texture  The texture to delete.
 */
void dmnsn_delete_texture(dmnsn_texture *texture);

/**
 * Initialize a texture.  Textures should not be used before being initialized,
 * but should not be modified after being initialized.  Textures are generally
 * initialized for you.
 * @param[in,out] texture  The texture to initialize.
 */
void dmnsn_initialize_texture(dmnsn_texture *texture);

/**
 * Fill missing texture properties from a default texture.
 * @param[in]     default_texture  The default texture.
 * @param[in,out] texturep         A pointer to the texture to fill.
 */
void dmnsn_texture_cascade(dmnsn_texture *default_texture,
                           dmnsn_texture **texturep);
