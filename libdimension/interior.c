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

#include "dimension.h"
#include <errno.h>
#include <stdlib.h> /* For malloc */

/* Allocate an interior */
dmnsn_interior *
dmnsn_new_interior()
{
  dmnsn_interior *interior = malloc(sizeof(dmnsn_interior));
  if (interior) {
    interior->ior     = 1.0;
    interior->free_fn = NULL;
  } else {
    errno = ENOMEM;
  }
  return interior;
}

/* Free a interior */
void
dmnsn_delete_interior(dmnsn_interior *interior)
{
  if (interior) {
    if (interior->free_fn) {
      (*interior->free_fn)(interior->ptr);
    }
    free(interior);
  }
}
