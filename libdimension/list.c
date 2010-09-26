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

#include "dimension.h"

dmnsn_list *
dmnsn_list_from_array(const dmnsn_array *array)
{
  dmnsn_list *list = dmnsn_new_list(array->obj_size);

  for (size_t i = 0; i < dmnsn_array_size(array); ++i) {
    dmnsn_list_push(list, dmnsn_array_at(array, i));
  }

  return list;
}

dmnsn_array *
dmnsn_array_from_list(const dmnsn_list *list)
{
  dmnsn_array *array = dmnsn_new_array(list->obj_size);

  for (dmnsn_list_iterator *i = dmnsn_list_first(list);
       i != NULL;
       i = dmnsn_list_next(i))
  {
    dmnsn_array_push(array, i->ptr);
  }

  return array;
}

void
dmnsn_delete_list(dmnsn_list *list)
{
  if (list) {
    while (list->first) {
      dmnsn_list_remove(list, list->first);
    }
    dmnsn_free(list);
  }
}

dmnsn_list *
dmnsn_list_split(dmnsn_list *list)
{
  dmnsn_list *half = dmnsn_new_list(list->obj_size);

  /* Find the halfway point */
  size_t i, max = dmnsn_list_size(list)/2;
  dmnsn_list_iterator *ii;
  for (i = 0, ii = dmnsn_list_last(list);
       i < max && ii;
       ++i, ii = dmnsn_list_prev(ii));

  if (ii && ii->next) {
    half->first = ii->next;
    half->last = list->last;
    list->last = ii;
    half->first->prev = NULL;
    list->last->next  = NULL;
  }

  list->length -= max;
  half->length = max;
  return half;
}

void
dmnsn_list_sort(dmnsn_list *list, dmnsn_list_comparator_fn *comparator)
{
  /* Recursive merge sort */
  if (dmnsn_list_size(list) < 2) {
    return;
  } else if (dmnsn_list_size(list) == 2) {
    if ((*comparator)(list->last, list->first)) {
      dmnsn_list_swap(list->first, list->last);
    }
  } else {
    dmnsn_list *half = dmnsn_list_split(list);
    dmnsn_list_sort(list, comparator);
    dmnsn_list_sort(half, comparator);

    dmnsn_list_iterator *i = list->first, *j = half->first;
    while (i || j) {
      if (!i) {
        j->prev = list->last;
        list->last->next = j;
        list->last = half->last;
        list->length += half->length;
        half->first = half->last = NULL;
        half->length = 0;
        break;
      } else if (!j) {
        break;
      } else if ((*comparator)(j, i)) {
        dmnsn_list_iterator *temp = dmnsn_list_next(j);
        dmnsn_list_iterator_remove(half, j);
        dmnsn_list_iterator_insert(list, i, j);
        j = temp;
      } else {
        i = dmnsn_list_next(i);
      }
    }

    dmnsn_delete_list(half);
  }
}
