/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Interiors.
 */

#include "dimension-internal.h"
#include <stdlib.h>

/* Allocate an interior */
dmnsn_interior *
dmnsn_new_interior(void)
{
  dmnsn_interior *interior = dmnsn_malloc(sizeof(dmnsn_interior));
  interior->ior     = 1.0;
  interior->free_fn = NULL;
  DMNSN_REFCOUNT_INIT(interior);
  return interior;
}

/* Free a interior */
void
dmnsn_delete_interior(dmnsn_interior *interior)
{
  if (DMNSN_DECREF(interior)) {
    if (interior->free_fn) {
      interior->free_fn(interior->ptr);
    }
    dmnsn_free(interior);
  }
}

/* Cascade a interior */
void
dmnsn_interior_cascade(dmnsn_interior *default_interior,
                       dmnsn_interior **interiorp)
{
  if (!*interiorp) {
    *interiorp = default_interior;
    DMNSN_INCREF(*interiorp);
  }
}
