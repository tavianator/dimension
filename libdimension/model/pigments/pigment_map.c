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
 * Pigment-mapped pigment patterns.
 */

#include "dimension/model.h"

/// Initialize a pigment in a pigment map.
static void
dmnsn_initialize_mapped_pigment(void *ptr)
{
  dmnsn_pigment **pigment = ptr;
  dmnsn_pigment_initialize(*pigment);
}

dmnsn_map *
dmnsn_new_pigment_map(dmnsn_pool *pool)
{
  return dmnsn_new_map(pool, sizeof(dmnsn_pigment *));
}

/// Pigment map type.
typedef struct dmnsn_pigment_map {
  dmnsn_pigment pigment;
  dmnsn_pattern *pattern;
  dmnsn_map *map;
  dmnsn_pigment_map_flags flags;
} dmnsn_pigment_map;

/// pigment_map pigment callback.
static dmnsn_tcolor
dmnsn_pigment_map_pigment_fn(const dmnsn_pigment *pigment, dmnsn_vector v)
{
  const dmnsn_pigment_map *pigment_map = (const dmnsn_pigment_map *)pigment;
  double n;
  dmnsn_pigment *pigment1, *pigment2;
  dmnsn_map_evaluate(pigment_map->map,
                     dmnsn_pattern_value(pigment_map->pattern, v),
                     &n, &pigment1, &pigment2);
  dmnsn_tcolor color1 = dmnsn_pigment_evaluate(pigment1, v);
  dmnsn_tcolor color2 = dmnsn_pigment_evaluate(pigment2, v);

  if (pigment_map->flags == DMNSN_PIGMENT_MAP_SRGB) {
    color1.c = dmnsn_color_to_sRGB(color1.c);
    color2.c = dmnsn_color_to_sRGB(color2.c);
  }
  dmnsn_tcolor ret = dmnsn_tcolor_gradient(color1, color2, n);
  if (pigment_map->flags == DMNSN_PIGMENT_MAP_SRGB) {
    ret.c = dmnsn_color_from_sRGB(ret.c);
  }

  return ret;
}

/// pigment_map initialization callback.
static void
dmnsn_pigment_map_initialize_fn(dmnsn_pigment *pigment)
{
  dmnsn_pigment_map *pigment_map = (dmnsn_pigment_map *)pigment;
  dmnsn_map_apply(pigment_map->map, dmnsn_initialize_mapped_pigment);
}

dmnsn_pigment *
dmnsn_new_pigment_map_pigment(dmnsn_pool *pool, dmnsn_pattern *pattern, dmnsn_map *map, dmnsn_pigment_map_flags flags)
{
  dmnsn_pigment_map *pigment_map = DMNSN_PALLOC(pool, dmnsn_pigment_map);
  pigment_map->pattern = pattern;
  pigment_map->map = map;
  pigment_map->flags = flags;

  dmnsn_pigment *pigment = &pigment_map->pigment;
  dmnsn_init_pigment(pigment);
  pigment->pigment_fn = dmnsn_pigment_map_pigment_fn;
  pigment->initialize_fn = dmnsn_pigment_map_initialize_fn;
  return pigment;
}
