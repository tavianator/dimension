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

/*
 * Object textures.
 */

#ifndef DIMENSION_TEXTURE_H
#define DIMENSION_TEXTURE_H

/*
 * Pigments
 */

/* Forward-declare dmnsn_pigment */
typedef struct dmnsn_pigment dmnsn_pigment;

/* Pigment callback */
typedef dmnsn_color dmnsn_pigment_fn(const dmnsn_pigment *pigment,
                                     dmnsn_vector v);

/* dmnsn_pigment definition */
struct dmnsn_pigment {
  dmnsn_pigment_fn *pigment_fn;
  void *ptr;
};

dmnsn_pigment *dmnsn_new_pigment();
void dmnsn_delete_pigment(dmnsn_pigment *pigment);

/*
 * A complete texture
 */

typedef struct {
  dmnsn_pigment *pigment;
} dmnsn_texture;

dmnsn_texture *dmnsn_new_texture();
void dmnsn_delete_texture(dmnsn_texture *texture);

#endif /* DIMENSION_TEXTURE_H */
