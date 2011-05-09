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
 * Simple doubly-linked lists.
 */

#ifndef DIMENSION_LIST_H
#define DIMENSION_LIST_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * A list iterator.
 */
typedef struct dmnsn_list_iterator {
  void *ptr;                        /**< @internal The stored object. */
  size_t obj_size;                  /**< @internal The object size. */
  struct dmnsn_list_iterator *prev; /**< @internal The previous iterator. */
  struct dmnsn_list_iterator *next; /**< @internal The next iterator. */
} dmnsn_list_iterator;

/**
 * @internal
 * Iterator allocation.
 * @param[in] obj       The location of the object to store in the iterator.
 * @param[in] obj_size  The size of the object to store in the iterator.
 */
DMNSN_INLINE dmnsn_list_iterator *
dmnsn_new_list_iterator(const void *obj, size_t obj_size)
{
  dmnsn_list_iterator *i
    = (dmnsn_list_iterator *)dmnsn_malloc(sizeof(dmnsn_list_iterator));
  i->obj_size = obj_size;
  i->prev = NULL;
  i->next = NULL;
  i->ptr  = dmnsn_malloc(obj_size);
  memcpy(i->ptr, obj, obj_size);
  return i;
}

/**
 * @internal
 * Iterator release.
 * @param[in,out] i  The iterator to free.
 */
DMNSN_INLINE void
dmnsn_delete_list_iterator(dmnsn_list_iterator *i)
{
  if (i) {
    dmnsn_free(i->ptr);
    dmnsn_free(i);
  }
}

/** A doubly-linked list. */
typedef struct dmnsn_list {
  dmnsn_list_iterator *first; /**< @internal The first iterator in the list. */
  dmnsn_list_iterator *last;  /**< @internal The last iterator in the list. */
  size_t length;              /**< @internal The size of the list. */
  size_t obj_size;            /**< @internal The size of list objects. */
} dmnsn_list;

/**
 * Allocate a list.
 * @param[in] obj_size  The size of the objects to be stored.
 * @return An empty list.
 */
DMNSN_INLINE dmnsn_list *
dmnsn_new_list(size_t obj_size)
{
  dmnsn_list *list = (dmnsn_list *)dmnsn_malloc(sizeof(dmnsn_list));
  list->first    = NULL;
  list->last     = NULL;
  list->obj_size = obj_size;
  list->length   = 0;
  return list;
}

/**
 * Delete a list.
 * @param[in,out] list  The list to delete.
 */
void dmnsn_delete_list(dmnsn_list *list);

/**
 * Copy a list.
 * @param[in] list  The list to copy.
 * @return A list containing the same elements as \p list.
 */
dmnsn_list *dmnsn_copy_list(const dmnsn_list *list);

/**
 * Construct a list from an array.
 * @param[in] array  The array to copy.
 * @return A list with the same contents as \p array.
 */
dmnsn_list *dmnsn_list_from_array(const dmnsn_array *array);

/**
 * Construct an array from a list.
 * @param[in] list  The list to copy.
 * @return An array with the same contents as \p list.
 */
dmnsn_array *dmnsn_array_from_list(const dmnsn_list *list);

/**
 * First element.
 * @param[in] list  The list to index.
 * @return An iterator to the first list element, or NULL if the list is empty.
 */
DMNSN_INLINE dmnsn_list_iterator *
dmnsn_list_first(const dmnsn_list *list)
{
  return list->first;
}

/**
 * Last element.
 * @param[in] list  The list to index.
 * @return An iterator to the last list element, or NULL if the list is empty.
 */
DMNSN_INLINE dmnsn_list_iterator *
dmnsn_list_last(const dmnsn_list *list)
{
  return list->last;
}

/**
 * Previous element.
 * @param[in] i  The iterator to follow.
 * @return The iterator preceding \c i.
 */
DMNSN_INLINE dmnsn_list_iterator *
dmnsn_list_prev(const dmnsn_list_iterator *i)
{
  dmnsn_assert(i, "NULL list iterator.");
  return i->prev;
}

/**
 * Next element.
 * @param[in] i  The iterator to follow.
 * @return The iterator following \c i.
 */
DMNSN_INLINE dmnsn_list_iterator *
dmnsn_list_next(const dmnsn_list_iterator *i)
{
  dmnsn_assert(i, "NULL list iterator.");
  return i->next;
}

/**
 * Get the size of the list.
 * @param[in] list  The list in question.
 * @return The number of elements in the list.
 */
DMNSN_INLINE size_t
dmnsn_list_size(const dmnsn_list *list)
{
  return list->length;
}

/**
 * Get the i'th object.
 * @param[in]  i   The iterator to dereference.
 * @param[out] obj The location to store the object.
 */
DMNSN_INLINE void
dmnsn_list_get(const dmnsn_list_iterator *i, void *obj)
{
  dmnsn_assert(i, "NULL list iterator.");
  memcpy(obj, i->ptr, i->obj_size);
}

/**
 * Get a pointer to the i'th object.
 * @param[in] i  The iterator to dereference.
 * @return A pointer to the object stored at \c i.
 */
DMNSN_INLINE void *
dmnsn_list_at(const dmnsn_list_iterator *i)
{
  dmnsn_assert(i, "NULL list iterator.");
  return i->ptr;
}

/**
 * Set the i'th object.
 * @param[in,out] i    The iterator to dereference.
 * @param[in]     obj  The object to store at \c i.
 */
DMNSN_INLINE void
dmnsn_list_set(dmnsn_list_iterator *i, const void *obj)
{
  dmnsn_assert(i, "NULL list iterator.");
  memcpy(i->ptr, obj, i->obj_size);
}

/**
 * Swap the objects in two iterators.
 * @param[in,out] a  The first iterator.
 * @param[in,out] b  The second iterator.
 */
DMNSN_INLINE void
dmnsn_list_swap(dmnsn_list_iterator *a, dmnsn_list_iterator *b)
{
  void *temp = a->ptr;
  a->ptr = b->ptr;
  b->ptr = temp;
}

/**
 * Insert a detached iterator into a list.
 * @param[in,out] list  The list to insert into.
 * @param[in,out] i     The iterator before which to insert, or NULL for the end
 *                      of the list.
 * @param[in,out] j     The detached iterator to insert.
 */
DMNSN_INLINE void
dmnsn_list_iterator_insert(dmnsn_list *list,
                           dmnsn_list_iterator *i, dmnsn_list_iterator *j)
{
  if (list->first == i)
    list->first = j;

  if (i) {
    j->prev = i->prev;
    i->prev = j;
  } else {
    j->prev = list->last;
    list->last = j;
  }

  if (j->prev)
    j->prev->next = j;
  j->next = i;
  ++list->length;
}

/**
 * Insert an object.
 * @param[in,out] list  The list to insert into.
 * @param[in,out] i     The iterator before which to insert, or NULL for the end
 *                      of the list.
 * @param[in]     obj   The location of the object to insert.
 */
DMNSN_INLINE void
dmnsn_list_insert(dmnsn_list *list, dmnsn_list_iterator *i, const void *obj)
{
  dmnsn_list_iterator *j = dmnsn_new_list_iterator(obj, list->obj_size);
  dmnsn_list_iterator_insert(list, i, j);
}

/**
 * Detach an iterator from a list.
 * @param[in,out] list  The list to remove from.
 * @param[in,out] i     The iterator to detach.
 */
DMNSN_INLINE void
dmnsn_list_iterator_remove(dmnsn_list *list, dmnsn_list_iterator *i)
{
  dmnsn_assert(i, "NULL list iterator.");
  dmnsn_assert(list->first, "List is empty.");
  if (list->first == i)
    list->first = i->next;
  if (list->last == i)
    list->last = i->prev;
  if (i->prev)
    i->prev->next = i->next;
  if (i->next)
    i->next->prev = i->prev;
  i->prev = NULL;
  i->next = NULL;
  --list->length;
}

/**
 * Remove the specified item from a list.
 * @param[in,out] list  The list to remove from.
 * @param[in,out] i     The iterator to delete.
 */
DMNSN_INLINE void
dmnsn_list_remove(dmnsn_list *list, dmnsn_list_iterator *i)
{
  dmnsn_list_iterator_remove(list, i);
  dmnsn_delete_list_iterator(i);
}

/**
 * Push an object to the end of the list.
 * @param[in,out] list  The list to append to.
 * @param[in]     obj   The location of the object to push.
 */
DMNSN_INLINE void
dmnsn_list_push(dmnsn_list *list, const void *obj)
{
  dmnsn_list_insert(list, NULL, obj);
}

/**
 * Pop an object from the end of the list.
 * @param[in,out] list  The list to extract from.
 * @param[out]    obj   The location to store the extracted object.
 */
DMNSN_INLINE void
dmnsn_list_pop(dmnsn_list *list, void *obj)
{
  dmnsn_assert(list->last, "List is empty.");
  dmnsn_list_get(list->last, obj);
  dmnsn_list_remove(list, list->last);
}

/**
 * Split a list in half, and return the second half.
 * @param[in,out] list  The list to split.
 * @return A the second half of the list.
 */
dmnsn_list *dmnsn_list_split(dmnsn_list *list);

/**
 * List object comparator function type.
 * @param[in] l  The first iterator.
 * @param[in] r  The second iterator.
 * @return Whether \p l < \p r.
 */
typedef bool dmnsn_list_comparator_fn(const dmnsn_list_iterator *l,
                                      const dmnsn_list_iterator *r);

/**
 * Sort a list, with O(n*log(n)) comparisons.
 * @param[in,out] list       The list to sort.
 * @param[in]     comparator The comparator to use for comparisons.
 */
void dmnsn_list_sort(dmnsn_list *list, dmnsn_list_comparator_fn *comparator);

#endif /* DIMENSION_LIST_H */
