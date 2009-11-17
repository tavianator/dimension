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

dmnsn_color
dmnsn_diffuse_finish_fn(const dmnsn_finish *finish,
                        dmnsn_color light, dmnsn_color color,
                        dmnsn_vector ray, dmnsn_vector normal,
                        dmnsn_vector viewer)
{
  double diffuse = dmnsn_vector_dot(ray, normal);
  dmnsn_color illum = dmnsn_color_illuminate(light, color);
  return dmnsn_color_mul(diffuse, illum);
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
