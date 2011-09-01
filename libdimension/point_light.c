/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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

/** Point light payload type. */
typedef struct dmnsn_point_light_payload {
  dmnsn_vector origin;
  dmnsn_color color;
} dmnsn_point_light_payload;

/** Point light direction callback. */
static dmnsn_vector
dmnsn_point_light_direction_fn(const dmnsn_light *light, dmnsn_vector v)
{
  dmnsn_point_light_payload *payload = light->ptr;
  return dmnsn_vector_sub(payload->origin, v);
}

/** Point light illumination callback. */
static dmnsn_color
dmnsn_point_light_illumination_fn(const dmnsn_light *light, dmnsn_vector v)
{
  dmnsn_point_light_payload *payload = light->ptr;
  return payload->color;
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
  dmnsn_light *light = dmnsn_new_light();

  dmnsn_point_light_payload *payload
    = dmnsn_malloc(sizeof(dmnsn_point_light_payload));
  payload->origin = x0;
  payload->color  = color;
  light->ptr = payload;

  light->direction_fn    = dmnsn_point_light_direction_fn;
  light->illumination_fn = dmnsn_point_light_illumination_fn;
  light->shadow_fn       = dmnsn_point_light_shadow_fn;
  light->free_fn         = dmnsn_free;

  return light;
}
