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

#include "dimension.h"
#include <stdlib.h>

/* Allocate a dummy pigment */
dmnsn_pigment *
dmnsn_new_pigment()
{
  dmnsn_pigment *pigment = dmnsn_malloc(sizeof(dmnsn_pigment));
  pigment->free_fn = NULL;
  pigment->trans   = dmnsn_identity_matrix();
  return pigment;
}

/* Free a pigment */
void
dmnsn_delete_pigment(dmnsn_pigment *pigment)
{
  if (pigment) {
    if (pigment->free_fn) {
      (*pigment->free_fn)(pigment->ptr);
    }
    free(pigment);
  }
}

/* Precompute pigment properties */
void
dmnsn_pigment_precompute(dmnsn_pigment *pigment)
{
  pigment->trans_inv = dmnsn_matrix_inverse(pigment->trans);
}

/* Allocate a dummy finish */
dmnsn_finish *
dmnsn_new_finish()
{
  dmnsn_finish *finish = dmnsn_malloc(sizeof(dmnsn_finish));
  finish->diffuse_fn    = NULL;
  finish->specular_fn   = NULL;
  finish->ambient_fn    = NULL;
  finish->reflection_fn = NULL;
  finish->free_fn       = NULL;
  return finish;
}

/* Free a finish */
void
dmnsn_delete_finish(dmnsn_finish *finish)
{
  if (finish) {
    if (finish->free_fn) {
      (*finish->free_fn)(finish->ptr);
    }
    free(finish);
  }
}

/* Allocate a dummy texture */
dmnsn_texture *
dmnsn_new_texture()
{
  dmnsn_texture *texture = dmnsn_malloc(sizeof(dmnsn_texture));
  texture->pigment   = NULL;
  texture->finish    = NULL;
  texture->trans     = dmnsn_identity_matrix();
  texture->refcount  = dmnsn_malloc(sizeof(unsigned int));
  *texture->refcount = 1;
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
      free(texture->refcount);
      free(texture);
    } else {
      --*texture->refcount;
    }
  }
}

/* Calculate matrix inverses */
void
dmnsn_texture_precompute(dmnsn_texture *texture)
{
  texture->trans_inv = dmnsn_matrix_inverse(texture->trans);
  if (texture->pigment) {
    texture->pigment->trans
      = dmnsn_matrix_mul(texture->trans, texture->pigment->trans);
    dmnsn_pigment_precompute(texture->pigment);
  }
}
