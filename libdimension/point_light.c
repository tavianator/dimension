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
 * Point lights.
 */

#include "dimension.h"
#include <stdlib.h>

/** Point light type. */
typedef struct dmnsn_point_light {
  dmnsn_light light;
  dmnsn_vector origin;
  dmnsn_color color;
} dmnsn_point_light;

/** Point light direction callback. */
static dmnsn_vector
dmnsn_point_light_direction_fn(const dmnsn_light *light, dmnsn_vector v)
{
  const dmnsn_point_light *point_light = (const dmnsn_point_light *)light;
  return dmnsn_vector_sub(point_light->origin, v);
}

/** Point light illumination callback. */
static dmnsn_color
dmnsn_point_light_illumination_fn(const dmnsn_light *light, dmnsn_vector v)
{
  const dmnsn_point_light *point_light = (const dmnsn_point_light *)light;
  return point_light->color;
}

/** Point light illumination callback. */
static bool
dmnsn_point_light_shadow_fn(const dmnsn_light *light, double t)
{
  return t < 1.0;
}

dmnsn_light *
dmnsn_new_point_light(dmnsn_vector x0, dmnsn_color color)
{
  dmnsn_point_light *point_light = DMNSN_MALLOC(dmnsn_point_light);
  point_light->origin = x0;
  point_light->color = color;

  dmnsn_light *light = &point_light->light;
  dmnsn_init_light(light);
  light->direction_fn = dmnsn_point_light_direction_fn;
  light->illumination_fn = dmnsn_point_light_illumination_fn;
  light->shadow_fn = dmnsn_point_light_shadow_fn;
  return light;
}
