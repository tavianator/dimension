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
 * Diffuse finish.
 */

#include "dimension.h"
#include <math.h>
#include <stdlib.h>

/** Diffuse finish callback. */
static dmnsn_color
dmnsn_lambertian_diffuse_fn(const dmnsn_diffuse *diffuse,
                            dmnsn_color light, dmnsn_color color,
                            dmnsn_vector ray, dmnsn_vector normal)
{
  double *coeff = diffuse->ptr;
  double diffuse_factor = fabs((*coeff)*dmnsn_vector_dot(ray, normal));
  return dmnsn_color_mul(diffuse_factor, dmnsn_color_illuminate(light, color));
}

dmnsn_diffuse *
dmnsn_new_lambertian(double diffuse)
{
  dmnsn_diffuse *lambertian = dmnsn_new_diffuse();

  double *param = DMNSN_MALLOC(double);
  *param = diffuse;

  lambertian->diffuse_fn = dmnsn_lambertian_diffuse_fn;
  lambertian->free_fn    = dmnsn_free;
  lambertian->ptr        = param;

  return lambertian;
}
