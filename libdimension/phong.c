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
#include <stdlib.h> /* For malloc */
#include <math.h>

/*
 * Phong finish
 */

static dmnsn_color
dmnsn_phong_specular_fn(const dmnsn_finish *finish,
                        dmnsn_color light, dmnsn_color color,
                        dmnsn_vector ray, dmnsn_vector normal,
                        dmnsn_vector viewer)
{
  double *params = finish->ptr;

  double specular = params[0];
  double exp      = params[1];

  dmnsn_vector proj = dmnsn_vector_mul(2*dmnsn_vector_dot(ray, normal), normal);
  dmnsn_vector reflected = dmnsn_vector_sub(proj, ray);

  double specular_factor = dmnsn_vector_dot(reflected, viewer);
  if (specular_factor < 0.0) {
    return dmnsn_black;
  }

  specular_factor = pow(specular_factor, exp);
  return dmnsn_color_mul(specular*specular_factor, light);
}

/* A phong finish */
dmnsn_finish *
dmnsn_new_phong_finish(double specular, double exp)
{
  dmnsn_finish *finish = dmnsn_new_finish();
  if (finish) {
    double *params = malloc(2*sizeof(double));
    if (!params) {
      dmnsn_delete_finish(finish);
      return NULL;
    }

    params[0] = specular;
    params[1] = exp;

    finish->ptr         = params;
    finish->specular_fn = &dmnsn_phong_specular_fn;
    finish->free_fn     = &free;
  }
  return finish;
}
