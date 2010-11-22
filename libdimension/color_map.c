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

/**
 * @file
 * Color-mapped pigment patterns.
 */

#include "dimension.h"

/** An [index, color] pair. */
typedef struct dmnsn_color_map_entry {
  double n;
  dmnsn_color color;
} dmnsn_color_map_entry;

dmnsn_color_map *
dmnsn_new_color_map(void)
{
  return dmnsn_new_array(sizeof(dmnsn_color_map_entry));
}

void
dmnsn_delete_color_map(dmnsn_color_map *map)
{
  dmnsn_delete_array(map);
}

void
dmnsn_add_color_map_entry(dmnsn_color_map *map, double n, dmnsn_color c)
{
  dmnsn_color_map_entry entry = { .n = n, .color = c };

  /* Sorted insertion */
  size_t i;
  dmnsn_color_map_entry *other = dmnsn_array_last(map);
  for (i = dmnsn_array_size(map); i-- > 0;) {
    if (other->n <= n)
      break;
  }

  dmnsn_array_insert(map, i + 1, &entry);
}

dmnsn_color
dmnsn_color_map_value(const dmnsn_color_map *map, double n)
{
  dmnsn_color_map_entry *entry = dmnsn_array_first(map);

  double n1, n2 = 0.0;
  dmnsn_color c1, c2 = entry->color;

  if (n < n2) {
    return c2;
  }

  for (; entry <= (dmnsn_color_map_entry *)dmnsn_array_last(map); ++entry) {
    n1 = n2;
    c1 = c2;

    n2 = entry->n;
    c2 = entry->color;

    if (n < n2) {
      return dmnsn_color_gradient(c1, c2, (n - n1)/(n2 - n1));
    }
  }

  return c2;
}

/** Payload for a color_map pigment. */
typedef struct dmnsn_color_map_payload {
  dmnsn_pattern *pattern;
  dmnsn_color_map *map;
} dmnsn_color_map_payload;

/** Free a color_map payload. */
static void
dmnsn_delete_color_map_payload(void *ptr)
{
  dmnsn_color_map_payload *payload = ptr;
  dmnsn_delete_color_map(payload->map);
  dmnsn_delete_pattern(payload->pattern);
  dmnsn_free(payload);
}

/** color_map pigment callback. */
static dmnsn_color
dmnsn_color_map_pigment_fn(const dmnsn_pigment *pigment, dmnsn_vector v)
{
  const dmnsn_color_map_payload *payload = pigment->ptr;
  return dmnsn_color_map_value(payload->map,
                               dmnsn_pattern_value(payload->pattern, v));
}

/** color_map initialization callback. */
static void
dmnsn_color_map_initialize_fn(dmnsn_pigment *pigment)
{
  dmnsn_color_map_payload *payload = pigment->ptr;
  payload->pattern->trans = dmnsn_matrix_mul(pigment->trans,
                                             payload->pattern->trans);
  dmnsn_initialize_pattern(payload->pattern);
}

dmnsn_pigment *
dmnsn_new_color_map_pigment(dmnsn_pattern *pattern, dmnsn_color_map *map)
{
  dmnsn_pigment *pigment = dmnsn_new_pigment();

  dmnsn_color_map_payload *payload
    = dmnsn_malloc(sizeof(dmnsn_color_map_payload));
  payload->pattern = pattern;
  payload->map     = map;

  pigment->pigment_fn    = &dmnsn_color_map_pigment_fn;
  pigment->initialize_fn = &dmnsn_color_map_initialize_fn;
  pigment->free_fn       = &dmnsn_delete_color_map_payload;
  pigment->ptr           = payload;
  return pigment;
}
