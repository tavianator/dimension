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
#include <math.h>

/*
 * Ambient finish
 */

static dmnsn_color
dmnsn_ambient_finish_fn(const dmnsn_finish *finish, dmnsn_color pigment)
{
  dmnsn_color *ambient = finish->ptr;
  return dmnsn_color_illuminate(*ambient, pigment);
}

dmnsn_finish *
dmnsn_new_ambient_finish(dmnsn_color ambient)
{
  dmnsn_finish *finish = dmnsn_new_finish();
  if (finish) {
    dmnsn_color *param = malloc(sizeof(dmnsn_color));
    if (!param) {
      dmnsn_delete_finish(finish);
      return NULL;
    }

    *param = ambient;
    finish->ptr        = param;
    finish->ambient_fn = &dmnsn_ambient_finish_fn;
    finish->free_fn    = &free;
  }
  return finish;
}
