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

#include "dimension.h"
#include <stdlib.h> /* For malloc */

/* Allocate a dummy pigment */
dmnsn_pigment *
dmnsn_new_pigment()
{
  dmnsn_pigment *pigment = malloc(sizeof(dmnsn_pigment));
  if (pigment) {
    pigment->free_fn = NULL;
  }
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

/* Allocate a dummy finish */
dmnsn_finish *
dmnsn_new_finish()
{
  dmnsn_finish *finish = malloc(sizeof(dmnsn_finish));
  if (finish) {
    finish->diffuse_fn    = NULL;
    finish->specular_fn   = NULL;
    finish->ambient_fn    = NULL;
    finish->reflection_fn = NULL;
    finish->free_fn       = NULL;
  }
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
  dmnsn_texture *texture = malloc(sizeof(dmnsn_texture));
  if (texture) {
    texture->pigment = NULL;
    texture->finish = NULL;
  }
  return texture;
}

/* Free a texture */
void
dmnsn_delete_texture(dmnsn_texture *texture)
{
  if (texture) {
    dmnsn_delete_finish(texture->finish);
    dmnsn_delete_pigment(texture->pigment);
    free(texture);
  }
}
