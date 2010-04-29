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
  if (!i)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "NULL list iterator.");
  return i->prev;
}

DMNSN_INLINE dmnsn_list_iterator *
dmnsn_list_next(dmnsn_list_iterator *i)
{
  if (!i)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "NULL list iterator.");
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
  if (!i)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "NULL list iterator.");
  memcpy(obj, i->ptr, i->obj_size);
}

/* Set the i'th object, expanding the list if necessary */
DMNSN_INLINE void
dmnsn_list_set(dmnsn_list_iterator *i, const void *obj)
{
  if (!i)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "NULL list iterator.");
  memcpy(i->ptr, obj, i->obj_size);
}

/* Insert an item before `i' */
DMNSN_INLINE void
dmnsn_list_insert(dmnsn_list *list, dmnsn_list_iterator *i, const void *obj)
{
  if (!i)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "NULL list iterator.");
  dmnsn_list_iterator *j = dmnsn_new_list_iterator(obj, list->obj_size);
  if (list->first == i)
    list->first = j;
  j->next = i;
  j->prev = i->prev;
  i->prev = j;
  ++list->length;
}

/* Remove the specified item */
DMNSN_INLINE void
dmnsn_list_remove(dmnsn_list *list, dmnsn_list_iterator *i)
{
  if (!i)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "NULL list iterator.");
  if (list->first == i)
    list->first = i->next;
  if (list->last == i)
    list->last = i->prev;
  if (i->prev)
    i->prev->next = i->next;
  if (i->next)
    i->next->prev = i->prev;
  dmnsn_delete_list_iterator(i);
  --list->length;
}

/* Push obj to the end of the list */
DMNSN_INLINE void
dmnsn_list_push(dmnsn_list *list, const void *obj)
{
  dmnsn_list_iterator *i = dmnsn_new_list_iterator(obj, list->obj_size);
  if (!list->first) {
    list->first = i;
  } else {
    i->prev = list->last;
    i->prev->next = i;
  }
  list->last = i;
  ++list->length;
}

/* Pop obj from the end of the list */
DMNSN_INLINE void
dmnsn_list_pop(dmnsn_list *list, void *obj)
{
  if (!list->last) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "List is empty.");
  }
  dmnsn_list_get(list->last, obj);
  dmnsn_list_remove(list, list->last);
}

#endif /* DIMENSION_LIST_H */
