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
 * Associative arrays, implemented with PATRICIA tries.
 */

#include "dimension.h"

struct dmnsn_dictionary {
  size_t obj_size;       /**< The size of the objects in the trie. */
  char *prefix;          /**< The local string prefix of the current node. */
  void *value;           /**< The node's stored object, if it's a leaf. */
  dmnsn_array *children; /**< The node's children. */
};

dmnsn_dictionary *
dmnsn_new_dictionary(size_t obj_size)
{
  dmnsn_dictionary *dict = DMNSN_MALLOC(dmnsn_dictionary);
  dict->obj_size = obj_size;
  dict->prefix   = dmnsn_strdup("");
  dict->value    = NULL;
  dict->children = dmnsn_new_array(sizeof(dmnsn_dictionary *));
  return dict;
}

void
dmnsn_delete_dictionary(dmnsn_dictionary *dict)
{
  if (dict) {
    dmnsn_free(dict->prefix);
    dmnsn_free(dict->value);
    DMNSN_ARRAY_FOREACH (dmnsn_dictionary **, subtrie, dict->children) {
      dmnsn_delete_dictionary(*subtrie);
    }
    dmnsn_delete_array(dict->children);

    dmnsn_free(dict);
  }
}

bool
dmnsn_dictionary_get(const dmnsn_dictionary *dict, const char *key, void *obj)
{
  const void *value = dmnsn_dictionary_at(dict, key);
  if (value) {
    memcpy(obj, value, dict->obj_size);
    return true;
  } else {
    return false;
  }
}

void *
dmnsn_dictionary_at(const dmnsn_dictionary *dict, const char *key)
{
  /*
   * PATRICIA trie search: O(k), where k is the length of the longest string
   * in the trie.
   */

  size_t len = strlen(dict->prefix);
  if (strncmp(key, dict->prefix, len) != 0)
    return NULL;
  key += len;

  while (true) {
    if (*key == '\0' && dict->value) {
      return dict->value;
    } else {
      dmnsn_dictionary **first = dmnsn_array_first(dict->children), **subtrie;
      ptrdiff_t size = dmnsn_array_size(dict->children);
      for (subtrie = first; subtrie - first < size; ++subtrie) {
        len = strlen((*subtrie)->prefix);
        if (strncmp(key, (*subtrie)->prefix, len) == 0) {
          dict = *subtrie;
          key += len;
          break;
        }
      }

      if (subtrie - first == size)
        break;
    }
  }

  return NULL;
}

void
dmnsn_dictionary_insert(dmnsn_dictionary *dict, const char *key,
                        const void *obj)
{
  /*
   * PATRICIA trie insertion: O(k), where k is the length of the longest string
   * in the trie.
   */

  while (true) {
    if (dict->prefix[0] == '\0' && !dict->value
        && dmnsn_array_size(dict->children) == 0)
    {
      /* Replace an empty tree with a single-element tree */
      dict->prefix = dmnsn_realloc(dict->prefix, strlen(key) + 1);
      strcpy(dict->prefix, key);

      dict->value = dmnsn_malloc(dict->obj_size);
      memcpy(dict->value, obj, dict->obj_size);
      break;
    }

    char *prefix = dict->prefix;
    while (*prefix == *key && *prefix && *key) {
      ++prefix;
      ++key;
    }

    if (*key == '\0' && *prefix == '\0') {
      /* Complete match */
      if (!dict->value) {
        dict->value = dmnsn_malloc(dict->obj_size);
      }
      memcpy(dict->value, obj, dict->obj_size);
      break;
    } else if (*prefix == '\0') {
      /* Partial match; key starts with prefix */
      dmnsn_dictionary **first = dmnsn_array_first(dict->children), **subtrie;
      ptrdiff_t size = dmnsn_array_size(dict->children);
      for (subtrie = first; subtrie - first < size; ++subtrie) {
        if ((*subtrie)->prefix[0] == key[0]) {
          dict = *subtrie;
          break;
        }
      }

      if (subtrie - first == size) {
        /* No submatch found, add a new child */
        dmnsn_dictionary *child = dmnsn_new_dictionary(dict->obj_size);
        dmnsn_array_push(dict->children, &child);
        dict = child;
      }
    } else {
      /* Split the tree */
      dmnsn_dictionary *copy = dmnsn_new_dictionary(dict->obj_size);
      copy->prefix = dmnsn_realloc(copy->prefix, strlen(prefix) + 1);
      strcpy(copy->prefix, prefix);
      *prefix = '\0';

      copy->value = dict->value;
      dict->value = NULL;

      dmnsn_array *temp = copy->children;
      copy->children = dict->children;
      dict->children = temp;

      dmnsn_dictionary *subtrie = dmnsn_new_dictionary(dict->obj_size);
      dmnsn_array_push(dict->children, &copy);
      dmnsn_array_push(dict->children, &subtrie);
      dict = subtrie;
    }
  }
}

bool
dmnsn_dictionary_remove(dmnsn_dictionary *dict, const char *key)
{
  /*
   * PATRICIA trie removal: O(k), where k is the length of the longest string
   * in the trie.
   *
   * This implementation doesn't actually collapse the tree back upwards if a
   * node is left with only one child, to reduce complexity.
   */

  size_t len = strlen(dict->prefix);
  if (strncmp(key, dict->prefix, len) != 0)
    return false;
  key += len;

  while (true) {
    if (*key == '\0' && dict->value) {
      dmnsn_free(dict->value);
      dict->value = NULL;
      return true;
    } else {
      dmnsn_dictionary **first = dmnsn_array_first(dict->children), **subtrie;
      ptrdiff_t size = dmnsn_array_size(dict->children);
      for (subtrie = first; subtrie - first < size; ++subtrie) {
        len = strlen((*subtrie)->prefix);
        if (strncmp(key, (*subtrie)->prefix, len) == 0) {
          dict = *subtrie;
          key += len;
          break;
        }
      }

      if (subtrie - first == size)
        break;
    }
  }

  return false;
}

void
dmnsn_dictionary_apply(dmnsn_dictionary *dict, dmnsn_callback_fn *callback)
{
  if (dict->value) {
    callback(dict->value);
  }

  DMNSN_ARRAY_FOREACH (dmnsn_dictionary **, subtrie, dict->children) {
    dmnsn_dictionary_apply(*subtrie, callback);
  }
}
