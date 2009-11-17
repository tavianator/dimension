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

static dmnsn_color
dmnsn_diffuse_finish_fn(const dmnsn_finish *finish,
                         dmnsn_color color, dmnsn_vector x0,
                         dmnsn_vector normal, dmnsn_vector reflected)
{
  return dmnsn_color_mul(dmnsn_vector_dot(normal, reflected), color);
}

/* A diffuse finish */
dmnsn_finish *
dmnsn_new_diffuse_finish()
{
  dmnsn_finish *finish = dmnsn_new_finish();
  if (finish) {
    finish->finish_fn = &dmnsn_diffuse_finish_fn;
  }
  return finish;
}
