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

dmnsn_color
dmnsn_phong_finish_fn(const dmnsn_finish *finish,
                      dmnsn_color light, dmnsn_color color,
                      dmnsn_vector ray, dmnsn_vector normal,
                      dmnsn_vector viewer)
{
  double *params = finish->ptr;

  double diffuse  = params[0];
  double specular = params[1];
  double exp      = params[2];

  /* Diffuse component */
  double diffuse_factor = diffuse*dmnsn_vector_dot(ray, normal);
  dmnsn_color diffuse_color
    = dmnsn_color_mul(diffuse_factor, dmnsn_color_illuminate(light, color));

  /* Specular component */

  dmnsn_vector proj = dmnsn_vector_mul(2*dmnsn_vector_dot(ray, normal), normal);
  dmnsn_vector reflected = dmnsn_vector_sub(proj, ray);

  double specular_factor
    = specular*pow(dmnsn_vector_dot(reflected, viewer), exp);
  dmnsn_color specular_color = dmnsn_color_mul(specular_factor, light);

  return dmnsn_color_add(diffuse_color, specular_color);
}

/* A phong finish */
dmnsn_finish *
dmnsn_new_phong_finish(double diffuse, double specular, double exp)
{
  dmnsn_finish *finish = dmnsn_new_finish();
  if (finish) {
    double *params = malloc(3*sizeof(double));
    if (!params) {
      dmnsn_delete_finish(finish);
      return NULL;
    }

    params[0] = diffuse;
    params[1] = specular;
    params[2] = exp;

    finish->ptr       = params;
    finish->finish_fn = &dmnsn_phong_finish_fn;
    finish->free_fn   = &free;
  }
  return finish;
}
