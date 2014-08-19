/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Phong highlights.
 */

#include "dimension/model.h"
#include <stdlib.h>

/// Phone specular type.
typedef struct dmnsn_phong {
  dmnsn_specular specular;
  double coeff;
  double exp;
} dmnsn_phong;

/// Phong specular highlight callback.
static dmnsn_color
dmnsn_phong_specular_fn(const dmnsn_specular *specular,
                        dmnsn_color light, dmnsn_color color,
                        dmnsn_vector ray, dmnsn_vector normal,
                        dmnsn_vector viewer)
{
  const dmnsn_phong *phong = (const dmnsn_phong *)specular;

  dmnsn_vector proj = dmnsn_vector_mul(2*dmnsn_vector_dot(ray, normal), normal);
  dmnsn_vector reflected = dmnsn_vector_sub(proj, ray);

  double specular_factor = dmnsn_vector_dot(reflected, viewer);
  if (specular_factor < 0.0) {
    return dmnsn_black;
  }

  specular_factor = pow(specular_factor, phong->exp);
  return dmnsn_color_mul(phong->coeff*specular_factor, light);
}

// A phong finish
dmnsn_specular *
dmnsn_new_phong(dmnsn_pool *pool, double coeff, double exp)
{
  dmnsn_phong *phong = DMNSN_PALLOC(pool, dmnsn_phong);
  phong->coeff = coeff;
  phong->exp = exp;

  dmnsn_specular *specular = &phong->specular;
  dmnsn_init_specular(specular);
  specular->specular_fn = dmnsn_phong_specular_fn;
  return specular;
}
