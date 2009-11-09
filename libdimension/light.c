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

/* Allocate a new dummy light */
dmnsn_light *
dmnsn_new_light()
{
  dmnsn_light *light = malloc(sizeof(dmnsn_light));
  if (light) {
    light->light_fn = NULL;
    light->free_fn  = NULL;
    light->ptr      = NULL;
  }
  return light;
}

/* Free a dummy light */
void
dmnsn_delete_light(dmnsn_light *light)
{
  if (light) {
    if (light->free_fn) {
      (*light->free_fn)(light->ptr);
    }
    free(light);
  }
}
