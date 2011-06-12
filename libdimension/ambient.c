/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@tavianator.com>          *
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
 * Ambient finish.
 */

#include "dimension.h"
#include <math.h>
#include <stdlib.h>

/** Ambient finish callback. */
static dmnsn_color
dmnsn_basic_ambient_fn(const dmnsn_ambient *ambient, dmnsn_color pigment)
{
  dmnsn_color *light = ambient->ptr;
  dmnsn_color ret = dmnsn_color_illuminate(*light, pigment);
  ret.trans  = 0.0;
  ret.filter = 0.0;
  return ret;
}

dmnsn_ambient *
dmnsn_new_basic_ambient(dmnsn_color ambient)
{
  dmnsn_ambient *basic = dmnsn_new_ambient();

  dmnsn_color *param = dmnsn_malloc(sizeof(dmnsn_color));
  *param = ambient;

  basic->ambient_fn = dmnsn_basic_ambient_fn;
  basic->free_fn    = dmnsn_free;
  basic->ptr        = param;
  return basic;
}
