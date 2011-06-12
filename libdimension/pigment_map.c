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
 * Pigment-mapped pigment patterns.
 */

#include "dimension.h"

/** Initialize a pigment in a pigment map. */
static void
dmnsn_initialize_mapped_pigment(void *ptr)
{
  dmnsn_pigment **pigment = ptr;
  dmnsn_initialize_pigment(*pigment);
}

/** Free a pigment in a pigment map. */
static void
dmnsn_delete_mapped_pigment(void *ptr)
{
  dmnsn_pigment **pigment = ptr;
  dmnsn_delete_pigment(*pigment);
}

dmnsn_map *
dmnsn_new_pigment_map(void)
{
  dmnsn_map *pigment_map = dmnsn_new_map(sizeof(dmnsn_pigment *));
  pigment_map->free_fn = dmnsn_delete_mapped_pigment;
  return pigment_map;
}

/** Payload for a pigment_map pigment. */
typedef struct dmnsn_pigment_map_payload {
  dmnsn_pattern *pattern;
  dmnsn_map *map;
  dmnsn_pigment_map_flags flags;
} dmnsn_pigment_map_payload;

/** Free a pigment_map payload. */
static void
dmnsn_delete_pigment_map_payload(void *ptr)
{
  dmnsn_pigment_map_payload *payload = ptr;
  dmnsn_delete_map(payload->map);
  dmnsn_delete_pattern(payload->pattern);
  dmnsn_free(payload);
}

/** pigment_map pigment callback. */
static dmnsn_color
dmnsn_pigment_map_pigment_fn(const dmnsn_pigment *pigment, dmnsn_vector v)
{
  const dmnsn_pigment_map_payload *payload = pigment->ptr;
  double n;
  dmnsn_pigment *pigment1, *pigment2;
  dmnsn_evaluate_map(payload->map, dmnsn_pattern_value(payload->pattern, v),
                     &n, &pigment1, &pigment2);
  dmnsn_color color1 = dmnsn_evaluate_pigment(pigment1, v);
  dmnsn_color color2 = dmnsn_evaluate_pigment(pigment2, v);

  if (payload->flags == DMNSN_PIGMENT_MAP_SRGB) {
    color1 = dmnsn_color_to_sRGB(color1);
    color2 = dmnsn_color_to_sRGB(color2);
  }
  dmnsn_color ret = dmnsn_color_gradient(color1, color2, n);
  if (payload->flags == DMNSN_PIGMENT_MAP_SRGB) {
    ret = dmnsn_color_from_sRGB(ret);
  }

  return ret;
}

/** pigment_map initialization callback. */
static void
dmnsn_pigment_map_initialize_fn(dmnsn_pigment *pigment)
{
  dmnsn_pigment_map_payload *payload = pigment->ptr;
  dmnsn_initialize_pattern(payload->pattern);
  dmnsn_map_apply(payload->map, dmnsn_initialize_mapped_pigment);
}

dmnsn_pigment *
dmnsn_new_pigment_map_pigment(dmnsn_pattern *pattern, dmnsn_map *map,
                              dmnsn_pigment_map_flags flags)
{
  dmnsn_pigment *pigment = dmnsn_new_pigment();

  dmnsn_pigment_map_payload *payload
    = dmnsn_malloc(sizeof(dmnsn_pigment_map_payload));
  payload->pattern = pattern;
  payload->map     = map;
  payload->flags   = flags;

  pigment->pigment_fn    = dmnsn_pigment_map_pigment_fn;
  pigment->initialize_fn = dmnsn_pigment_map_initialize_fn;
  pigment->free_fn       = dmnsn_delete_pigment_map_payload;
  pigment->ptr           = payload;
  return pigment;
}
