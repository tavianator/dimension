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
 * Finishes.
 */

#include "dimension.h"

/* Allocate a dummy finish */
dmnsn_finish *
dmnsn_new_finish(void)
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
    dmnsn_free(finish);
  }
}
