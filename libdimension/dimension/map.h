/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Generic maps (backend for color_maps, pigment_maps, etc.).
 */

/** A map. */
typedef struct dmnsn_map {
  dmnsn_free_fn *free_fn; /**< Destructor callback. */
  size_t obj_size; /**< @internal The size of the mapped objects. */
  dmnsn_array *array; /**< @internal The map entries. */
} dmnsn_map;

/**
 * Create an empty map.
 * @param[in] size  The size of the objects to store in the map.
 * @return A map with no entries.
 */
dmnsn_map *dmnsn_new_map(size_t size);

/**
 * Delete a map.
 * @param[in,out] map  The map to delete.
 */
void dmnsn_delete_map(dmnsn_map *map);

/**
 * Add an entry (a scalar-object pair) to a color map.
 * @param[in,out] map  The color map to add to.
 * @param[in]     n    The index of the entry.
 * @param[in]     obj  The value of the entry.
 */
void dmnsn_add_map_entry(dmnsn_map *map, double n, const void *obj);

/**
 * Return the number of entries in a map.
 * @param[in] map  The map to measure.
 * @return The size of \p map.
 */
size_t dmnsn_map_size(const dmnsn_map *map);

/**
 * Evaluate a map.
 * @param[in]  map   The map to evaluate.
 * @param[in]  n     The index to evaluate.
 * @param[out] val   The normalized distance of \p n from \p obj1.
 * @param[out] obj1  The first object.
 * @param[out] obj2  The second object.
 */
void dmnsn_evaluate_map(const dmnsn_map *map, double n,
                        double *val, void *obj1, void *obj2);

/**
 * Apply a callback to each element of a map.
 * @param[in,out] map       The map.
 * @param[in]     callback  The callback to apply to the elements.
 */
void dmnsn_map_apply(dmnsn_map *map, dmnsn_callback_fn *callback);
