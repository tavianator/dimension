/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Lights.
 */

#include "dimension-internal.h"
#include <stdlib.h>

/* Allocate a new dummy light */
dmnsn_light *
dmnsn_new_light(void)
{
  dmnsn_light *light = DMNSN_MALLOC(dmnsn_light);
  light->direction_fn    = NULL;
  light->illumination_fn = NULL;
  light->shadow_fn       = NULL;
  light->free_fn         = NULL;
  light->ptr             = NULL;
  DMNSN_REFCOUNT_INIT(light);
  return light;
}

/* Free a dummy light */
void
dmnsn_delete_light(dmnsn_light *light)
{
  if (DMNSN_DECREF(light)) {
    if (light->free_fn) {
      light->free_fn(light->ptr);
    }
    dmnsn_free(light);
  }
}
