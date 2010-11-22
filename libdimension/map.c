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
 * Generic maps.
 */

#include "dimension.h"
#include <alloca.h>

/** An [index, object] pair. */
typedef struct dmnsn_map_entry {
  double n;
  char object[];
} dmnsn_map_entry;

dmnsn_map *
dmnsn_new_map(size_t size)
{
  dmnsn_map *map = dmnsn_malloc(sizeof(dmnsn_map));
  map->free_fn = NULL;
  map->obj_size = size;
  map->array = dmnsn_new_array(sizeof(dmnsn_map_entry) + size);
  return map;
}

void
dmnsn_delete_map(dmnsn_map *map)
{
  if (map) {
    if (map->free_fn) {
      for (size_t i = 0; i < dmnsn_array_size(map->array); ++i) {
        dmnsn_map_entry *entry = dmnsn_array_at(map->array, i);
        (*map->free_fn)(entry->object);
      }
    }

    dmnsn_delete_array(map->array);
    dmnsn_free(map);
  }
}

void
dmnsn_add_map_entry(dmnsn_map *map, double n, const void *obj)
{
  dmnsn_map_entry *entry = alloca(sizeof(dmnsn_map_entry) + map->obj_size);
  entry->n = n;
  memcpy(entry->object, obj, map->obj_size);

  /* Sorted insertion */
  size_t i;
  for (i = dmnsn_array_size(map->array); i-- > 0;) {
    dmnsn_map_entry *other = dmnsn_array_at(map->array, i);
    if (other->n <= n)
      break;
  }

  dmnsn_array_insert(map->array, i + 1, entry);
}

size_t
dmnsn_map_size(const dmnsn_map *map)
{
  return dmnsn_array_size(map->array);
}

void
dmnsn_evaluate_map(const dmnsn_map *map, double n,
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
