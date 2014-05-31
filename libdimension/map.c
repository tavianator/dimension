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
 * Generic maps.
 */

#include "dimension.h"

/** dmnsn_map definition. */
struct dmnsn_map {
  size_t obj_size; /**< @internal The size of the mapped objects. */
  dmnsn_array *array; /**< @internal The map entries. */
};

/** An [index, object] pair. */
typedef struct dmnsn_map_entry {
  double n;
  char object[];
} dmnsn_map_entry;

dmnsn_map *
dmnsn_new_map(dmnsn_pool *pool, size_t size)
{
  dmnsn_map *map = DMNSN_PALLOC(pool, dmnsn_map);
  map->obj_size = size;
  map->array = dmnsn_palloc_array(pool, sizeof(dmnsn_map_entry) + size);
  return map;
}

void
dmnsn_map_add_entry(dmnsn_map *map, double n, const void *obj)
{
  char mem[sizeof(dmnsn_map_entry) + map->obj_size];
  dmnsn_map_entry *entry = (dmnsn_map_entry *)mem;
  entry->n = n;
  memcpy(entry->object, obj, map->obj_size);

  /* Sorted insertion */
  size_t i;
  for (i = dmnsn_array_size(map->array); i-- > 0;) {
    dmnsn_map_entry *other = dmnsn_array_at(map->array, i);
    if (other->n <= n) {
      break;
    }
  }

  dmnsn_array_insert(map->array, i + 1, entry);
}

size_t
dmnsn_map_size(const dmnsn_map *map)
{
  return dmnsn_array_size(map->array);
}

void
dmnsn_map_evaluate(const dmnsn_map *map, double n,
                   double *val, void *obj1, void *obj2)
{
  dmnsn_assert(dmnsn_array_size(map->array) > 0,
               "Attempt to evaluate empty map.");

  const dmnsn_map_entry *entry = dmnsn_array_first(map->array);

  double n1, n2 = 0.0;
  const void *o1, *o2 = entry->object;

  if (n < n2) {
    *val = 0.0;
    memcpy(obj1, o2, map->obj_size);
    memcpy(obj2, o2, map->obj_size);
    return;
  }

  const dmnsn_map_entry *last = dmnsn_array_last(map->array);
  ptrdiff_t skip = sizeof(dmnsn_map_entry) + map->obj_size;
  for (; entry <= last;
       entry = (const dmnsn_map_entry *)((const char *)entry + skip))
  {
    n1 = n2;
    o1 = o2;

    n2 = entry->n;
    o2 = entry->object;

    if (n < n2) {
      *val = (n - n1)/(n2 - n1);
      memcpy(obj1, o1, map->obj_size);
      memcpy(obj2, o2, map->obj_size);
      return;
    }
  }

  *val = 1.0;
  memcpy(obj1, o2, map->obj_size);
  memcpy(obj2, o2, map->obj_size);
}

void
dmnsn_map_apply(dmnsn_map *map, dmnsn_callback_fn *callback)
{
  for (size_t i = 0; i < dmnsn_array_size(map->array); ++i) {
    dmnsn_map_entry *entry = dmnsn_array_at(map->array, i);
    callback(entry->object);
  }
}
