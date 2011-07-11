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
 * Reflective finish.
 */

#include "dimension.h"
#include <math.h>
#include <stdlib.h>

/** Reflective finish payload. */
typedef struct dmnsn_reflection_params {
  dmnsn_color min, max;
  double falloff;
} dmnsn_reflection_params;

/** Reflective finish callback. */
static dmnsn_color
dmnsn_basic_reflection_fn(const dmnsn_reflection *reflection,
                          dmnsn_color reflect, dmnsn_color color,
                          dmnsn_vector ray, dmnsn_vector normal)
{
  dmnsn_reflection_params *params = reflection->ptr;
  double coeff = pow(fabs(dmnsn_vector_dot(ray, normal)), params->falloff);

  return dmnsn_color_illuminate(
    dmnsn_color_gradient(params->min, params->max, coeff),
    reflect
  );
}

dmnsn_reflection *
dmnsn_new_basic_reflection(dmnsn_color min, dmnsn_color max, double falloff)
{
  dmnsn_reflection *reflection = dmnsn_new_reflection();

  dmnsn_reflection_params *params
    = dmnsn_malloc(sizeof(dmnsn_reflection_params));
  params->min     = min;
  params->max     = max;
  params->falloff = falloff;

  reflection->reflection_fn = dmnsn_basic_reflection_fn;
  reflection->free_fn       = dmnsn_free;
  reflection->ptr           = params;

  return reflection;
}