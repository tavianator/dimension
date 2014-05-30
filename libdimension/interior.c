/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
dmnsn_new_interior(dmnsn_pool *pool)
{
  dmnsn_interior *interior = DMNSN_PALLOC(pool, dmnsn_interior);
  interior->ior = 1.0;
  return interior;
}

/* Cascade a interior */
void
dmnsn_interior_cascade(dmnsn_interior *default_interior,
                       dmnsn_interior **interiorp)
{
  if (!*interiorp) {
    *interiorp = default_interior;
  }
}
