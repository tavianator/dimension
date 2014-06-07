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
 * Reflective finish.
 */

#include "dimension.h"
#include <math.h>
#include <stdlib.h>

/// Basic reflective finish type.
typedef struct dmnsn_basic_reflection {
  dmnsn_reflection reflection;
  dmnsn_color min, max;
  double falloff;
} dmnsn_basic_reflection;

/// Reflective finish callback.
static dmnsn_color
dmnsn_basic_reflection_fn(const dmnsn_reflection *reflection,
                          dmnsn_color reflect, dmnsn_color color,
                          dmnsn_vector ray, dmnsn_vector normal)
{
  const dmnsn_basic_reflection *basic = (const dmnsn_basic_reflection *)reflection;
  double coeff = pow(fabs(dmnsn_vector_dot(ray, normal)), basic->falloff);

  return dmnsn_color_illuminate(
    dmnsn_color_gradient(basic->min, basic->max, coeff),
    reflect
  );
}

dmnsn_reflection *
dmnsn_new_basic_reflection(dmnsn_pool *pool, dmnsn_color min, dmnsn_color max, double falloff)
{
  dmnsn_basic_reflection *basic = DMNSN_PALLOC(pool, dmnsn_basic_reflection);
  basic->min = min;
  basic->max = max;
  basic->falloff = falloff;

  dmnsn_reflection *reflection = &basic->reflection;
  dmnsn_init_reflection(reflection);
  reflection->reflection_fn = dmnsn_basic_reflection_fn;
  return reflection;
}
