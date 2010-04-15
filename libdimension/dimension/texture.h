/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
  /* Callbacks */
  dmnsn_pigment_fn *pigment_fn;
  dmnsn_free_fn *free_fn;

  /* Transformation matrix */
  dmnsn_matrix trans, trans_inv;

  /* Generic pointer */
  void *ptr;
};

dmnsn_pigment *dmnsn_new_pigment();
void dmnsn_delete_pigment(dmnsn_pigment *pigment);

void dmnsn_pigment_precompute(dmnsn_pigment *pigment);

/*
 * Finishes
 */

/* Forward-declare dmnsn_finish */
typedef struct dmnsn_finish dmnsn_finish;

/* Finish callbacks */
typedef dmnsn_color dmnsn_diffuse_fn(const dmnsn_finish *finish,
                                     dmnsn_color light, dmnsn_color color,
                                     dmnsn_vector ray, dmnsn_vector normal);
typedef dmnsn_color dmnsn_specular_fn(const dmnsn_finish *finish,
                                      dmnsn_color light, dmnsn_color color,
                                      dmnsn_vector ray, dmnsn_vector normal,
                                      dmnsn_vector viewer);
typedef dmnsn_color dmnsn_ambient_fn(const dmnsn_finish *finish,
                                     dmnsn_color pigment);
typedef dmnsn_color dmnsn_reflection_fn(const dmnsn_finish *finish,
                                        dmnsn_color reflect, dmnsn_color color,
                                        dmnsn_vector ray, dmnsn_vector normal);

/* dmnsn_finish definition */
struct dmnsn_finish {
  /* Callbacks */
  dmnsn_diffuse_fn    *diffuse_fn;
  dmnsn_specular_fn   *specular_fn;
  dmnsn_ambient_fn    *ambient_fn;
  dmnsn_reflection_fn *reflection_fn;
  dmnsn_free_fn       *free_fn;

  /* Generic pointer */
  void *ptr;
};

dmnsn_finish *dmnsn_new_finish();
void dmnsn_delete_finish(dmnsn_finish *finish);

/*
 * A complete texture
 */

typedef struct {
  /* Texture components */
  dmnsn_pigment *pigment;
  dmnsn_finish  *finish;

  /* Transformation matrix */
  dmnsn_matrix trans, trans_inv;
} dmnsn_texture;

dmnsn_texture *dmnsn_new_texture();
void dmnsn_delete_texture(dmnsn_texture *texture);

void dmnsn_texture_precompute(dmnsn_texture *texture);

#endif /* DIMENSION_TEXTURE_H */
