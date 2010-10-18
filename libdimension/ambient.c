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
#include <math.h>
#include <stdlib.h>

/*
 * Ambient finish
 */

static dmnsn_color
dmnsn_ambient_finish_fn(const dmnsn_finish *finish, dmnsn_color pigment)
{
  dmnsn_color *ambient = finish->ptr;
  dmnsn_color ret = dmnsn_color_illuminate(*ambient, pigment);
  ret.filter = 0.0;
  ret.trans  = 0.0;
  return ret;
}

dmnsn_finish *
dmnsn_new_ambient_finish(dmnsn_color ambient)
{
  dmnsn_finish *finish = dmnsn_new_finish();

  dmnsn_color *param = dmnsn_malloc(sizeof(dmnsn_color));
  *param = ambient;

  finish->ptr        = param;
  finish->ambient_fn = &dmnsn_ambient_finish_fn;
  finish->free_fn    = &dmnsn_free;

  return finish;
}
