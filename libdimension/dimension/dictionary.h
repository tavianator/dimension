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
 * Simple associative arrays.
 */

/// A string-object associative array.
typedef struct dmnsn_dictionary dmnsn_dictionary;

/**
 * Allocate a dictionary.
 * @param[in] obj_size  The size of the objects to store in the dictionary.
 * @return An empty dictionary.
 */
dmnsn_dictionary *dmnsn_new_dictionary(size_t obj_size);

/**
 * Delete a dictionary.
 * @param[in,out] dict  The dictionary to delete.
 */
void dmnsn_delete_dictionary(dmnsn_dictionary *dict);

/**
 * Find an element in a dictionary.
 * @param[in]  dict  The dictionary to search.
 * @param[in]  key   The key to search for.
 * @param[out] obj   The location to store the found object.
 * @return Whether the element was found.
 */
bool dmnsn_dictionary_get(const dmnsn_dictionary *dict, const char *key,
                          void *obj);

/**
 * Access an element in a dictionary.
 * @param[in]  dict  The dictionary to search.
 * @param[in]  key   The key to search for.
 * @return A pointer to the element if found, otherwise NULL.
 */
void *dmnsn_dictionary_at(const dmnsn_dictionary *dict, const char *key);

/**
 * Insert a (key, value) pair into a dictionary.
 * @param[in,out] dict  The dictionary to modify.
 * @param[in]     key   The key to insert.
 * @param[in]     obj   The object to insert.
 */
void dmnsn_dictionary_insert(dmnsn_dictionary *dict, const char *key,
                             const void *obj);

/**
 * Remove a (key, value) pair from a dictionary.
 * @param[in,out] dict  The dictionary to modify.
 * @param[in]     key   The key to remove.
 * @return Whether the key existed in the dictionary.
 */
bool dmnsn_dictionary_remove(dmnsn_dictionary *dict, const char *key);

/**
 * Apply a callback to all elements in a dictionary.
 * @param[in,out] dict      The dictionary.
 * @param[in]     callback  The callback to apply to the elements.
 */
void dmnsn_dictionary_apply(dmnsn_dictionary *dict,
                            dmnsn_callback_fn *callback);
