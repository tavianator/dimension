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
 * Textures.
 */

#include "dimension.h"

/* Allocate a dummy texture */
dmnsn_texture *
dmnsn_new_texture(void)
{
  dmnsn_texture *texture = dmnsn_malloc(sizeof(dmnsn_texture));
  texture->pigment     = NULL;
  texture->finish      = NULL;
  texture->trans       = dmnsn_identity_matrix();
  texture->refcount    = dmnsn_malloc(sizeof(unsigned int));
  *texture->refcount   = 1;
  texture->should_init = true;
  return texture;
}

/* Free a texture */
void
dmnsn_delete_texture(dmnsn_texture *texture)
{
  if (texture) {
    if (*texture->refcount <= 1) {
      dmnsn_delete_finish(texture->finish);
      dmnsn_delete_pigment(texture->pigment);
      dmnsn_free(texture->refcount);
      dmnsn_free(texture);
    } else {
      --*texture->refcount;
    }
  }
}

/* Calculate matrix inverses */
void
dmnsn_initialize_texture(dmnsn_texture *texture)
{
  texture->trans_inv = dmnsn_matrix_inverse(texture->trans);
  if (texture->pigment) {
    texture->pigment->trans
      = dmnsn_matrix_mul(texture->trans, texture->pigment->trans);
    dmnsn_initialize_pigment(texture->pigment);
  }
}
