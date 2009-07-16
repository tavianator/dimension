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

/* Allocate an dummy pigment */
dmnsn_pigment *
dmnsn_new_pigment()
{
  dmnsn_pigment *pigment = malloc(sizeof(dmnsn_pigment));
  if (pigment) {
    pigment->free_fn = NULL;
  }
  return pigment;
}

/* Free a dummy pigment */
void
dmnsn_delete_pigment(dmnsn_pigment *pigment)
{
  free(pigment);
}

/* Allocate a dummy texture */
dmnsn_texture *
dmnsn_new_texture()
{
  dmnsn_texture *texture = malloc(sizeof(dmnsn_texture));
  if (texture) {
    texture->pigment = NULL;
  }
  return texture;
}

/* Free a dummy texture */
void
dmnsn_delete_texture(dmnsn_texture *texture)
{
  free(texture);
}
