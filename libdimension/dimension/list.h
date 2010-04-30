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

/*
 * Simple generalized doubly-linked lists.
 */

#ifndef DIMENSION_LIST_H
#define DIMENSION_LIST_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct dmnsn_list_iterator {
  void *ptr;
  size_t obj_size;
  struct dmnsn_list_iterator *prev, *next;
} dmnsn_list_iterator;

/* Internal iterator allocation */

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

DMNSN_INLINE void
dmnsn_delete_list_iterator(dmnsn_list_iterator *i)
{
  if (i) {
    free(i->ptr);
    free(i);
  }
}

typedef struct dmnsn_list {
  dmnsn_list_iterator *first, *last;
  size_t obj_size, length;
} dmnsn_list;

/* List allocation */
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

/* Construction from arrays */
dmnsn_list *dmnsn_list_from_array(const dmnsn_array *array);
/* Delete a list */
void dmnsn_delete_list(dmnsn_list *list);

DMNSN_INLINE dmnsn_list_iterator *
dmnsn_list_first(dmnsn_list *list)
{
  return list->first;
}

DMNSN_INLINE dmnsn_list_iterator *
dmnsn_list_last(dmnsn_list *list)
{
  return list->last;
}

DMNSN_INLINE dmnsn_list_iterator *
dmnsn_list_prev(dmnsn_list_iterator *i)
{
  dmnsn_assert(i, "NULL list iterator.");
  return i->prev;
}

DMNSN_INLINE dmnsn_list_iterator *
dmnsn_list_next(dmnsn_list_iterator *i)
{
  dmnsn_assert(i, "NULL list iterator.");
  return i->next;
}

DMNSN_INLINE size_t
dmnsn_list_size(dmnsn_list *list)
{
  return list->length;
}

/* Get the i'th object */
DMNSN_INLINE void
dmnsn_list_get(const dmnsn_list_iterator *i, void *obj)
{
  dmnsn_assert(i, "NULL list iterator.");
  memcpy(obj, i->ptr, i->obj_size);
}

/* Set the i'th object, expanding the list if necessary */
DMNSN_INLINE void
dmnsn_list_set(dmnsn_list_iterator *i, const void *obj)
{
  dmnsn_assert(i, "NULL list iterator.");
  memcpy(i->ptr, obj, i->obj_size);
}

/* Swap two iterators */
DMNSN_INLINE void
dmnsn_list_swap(dmnsn_list_iterator *a, dmnsn_list_iterator *b)
{
  dmnsn_list_iterator temp = *a;
  *a = *b;
  *b = temp;
}

/* Insert `j' before `i' */
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
    if (list->last) {
      j->prev = list->last;
      j->prev->next = j;
    }
    list->last = j;
  }

  j->next = i;
  ++list->length;
}

/* Insert an item before `i' (NULL means at the end) */
DMNSN_INLINE void
dmnsn_list_insert(dmnsn_list *list, dmnsn_list_iterator *i, const void *obj)
{
  dmnsn_list_iterator *j = dmnsn_new_list_iterator(obj, list->obj_size);
  dmnsn_list_iterator_insert(list, i, j);
}

/* Remove the given iterator */
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
  --list->length;
}

/* Remove the specified item */
DMNSN_INLINE void
dmnsn_list_remove(dmnsn_list *list, dmnsn_list_iterator *i)
{
  dmnsn_list_iterator_remove(list, i);
  dmnsn_delete_list_iterator(i);
}

/* Push obj to the end of the list */
DMNSN_INLINE void
dmnsn_list_push(dmnsn_list *list, const void *obj)
{
  dmnsn_list_insert(list, NULL, obj);
}

/* Pop obj from the end of the list */
DMNSN_INLINE void
dmnsn_list_pop(dmnsn_list *list, void *obj)
{
  dmnsn_assert(list->last, "List is empty.");
  dmnsn_list_get(list->last, obj);
  dmnsn_list_remove(list, list->last);
}

/* Splits a list in half, and returns the second half */
dmnsn_list *dmnsn_list_split(dmnsn_list *list);
/* Sort a list */
typedef bool dmnsn_comparator_fn(dmnsn_list_iterator *l,
                                 dmnsn_list_iterator *r);
void dmnsn_list_sort(dmnsn_list *list, dmnsn_comparator_fn *comparator);

#endif /* DIMENSION_LIST_H */
