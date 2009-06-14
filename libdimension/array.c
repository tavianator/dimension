/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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
#include <string.h> /* For memcpy */

dmnsn_array *
dmnsn_new_array(size_t obj_size)
{
  dmnsn_array *array = malloc(sizeof(dmnsn_array));
  if (array) {
    array->obj_size = obj_size;
    array->length   = 0;
    array->capacity = 4; /* Start with capacity of 4 */

    array->ptr = malloc(array->capacity*array->obj_size);
    if (!array->ptr) {
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Array allocation failed.");
    }
  }

  return array;
}

void
dmnsn_array_push(dmnsn_array *array, const void *obj)
{
  dmnsn_array_set(array, array->length, obj);
}


void
dmnsn_array_pop(dmnsn_array *array, void *obj)
{
  dmnsn_array_get(array, array->length - 1, obj);
  dmnsn_array_resize(array, array->length - 1);
}

void *
dmnsn_array_at(dmnsn_array *array, size_t i)
{
  if (i >= array->length) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Array index out of bounds.");
  }
  return array->ptr + array->obj_size*i;
}

void
dmnsn_array_get(const dmnsn_array *array, size_t i, void *obj)
{
  if (i >= array->length) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Array index out of bounds.");
  }
  memcpy(obj, array->ptr + array->obj_size*i, array->obj_size);
}

void
dmnsn_array_set(dmnsn_array *array, size_t i, const void *obj)
{
  if (i >= array->length) {
    dmnsn_array_resize(array, i + 1);
  }
  memcpy(array->ptr + array->obj_size*i, obj, array->obj_size);
}

void
dmnsn_array_resize(dmnsn_array *array, size_t length)
{
  if (length > array->capacity) {
    array->capacity = length*2; /* We are greedy */
    array->ptr = realloc(array->ptr, array->obj_size*array->capacity);
    if (!array->ptr) {
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Resizing array failed.");
    }
  }

  array->length = length;
}

void dmnsn_delete_array(dmnsn_array *array) {
  if (array) {
    free(array->ptr);
    free(array);
  }
}
