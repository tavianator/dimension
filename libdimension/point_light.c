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

/**
 * @file
 * Point lights.
 */

#include "dimension.h"
#include <stdlib.h>

/** Point light callback. */
static dmnsn_color
dmnsn_point_light_fn(const dmnsn_light *light, dmnsn_vector v)
{
  return *(dmnsn_color *)light->ptr;
}

dmnsn_light *
dmnsn_new_point_light(dmnsn_vector x0, dmnsn_color color)
{
  dmnsn_light *light = dmnsn_new_light();

  dmnsn_color *ptr = dmnsn_malloc(sizeof(dmnsn_color));
  *ptr = color;

  light->x0       = x0;
  light->light_fn = &dmnsn_point_light_fn;
  light->free_fn  = &dmnsn_free;
  light->ptr      = ptr;

  return light;
}
