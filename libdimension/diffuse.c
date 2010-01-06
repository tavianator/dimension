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
 * Diffuse finish
 */

static dmnsn_color
dmnsn_diffuse_finish_fn(const dmnsn_finish *finish,
                        dmnsn_color light, dmnsn_color color,
                        dmnsn_vector ray, dmnsn_vector normal,
                        dmnsn_vector viewer)
{
  double *diffuse = finish->ptr;
  double diffuse_factor = (*diffuse)*dmnsn_vector_dot(ray, normal);
  return dmnsn_color_mul(diffuse_factor, dmnsn_color_illuminate(light, color));
}

dmnsn_finish *
dmnsn_new_diffuse_finish(double diffuse)
{
  dmnsn_finish *finish = dmnsn_new_finish();
  if (finish) {
    double *param = malloc(sizeof(double));
    if (!param) {
      dmnsn_delete_finish(finish);
      return NULL;
    }

    *param = diffuse;

    finish->ptr       = param;
    finish->finish_fn = &dmnsn_diffuse_finish_fn;
    finish->free_fn   = &free;
  }
  return finish;
}
