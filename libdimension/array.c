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
#include <pthread.h>
#include <string.h> /* For memcpy */

/* The raw implementations, which don't do any thread synchronicity */
static void dmnsn_array_get_impl(const dmnsn_array *array, size_t i, void *obj);
static void dmnsn_array_set_impl(dmnsn_array *array, size_t i, const void *obj);

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

    array->rwlock = malloc(sizeof(pthread_rwlock_t));
    if (!array->rwlock || pthread_rwlock_init(array->rwlock, NULL) != 0) {
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Array rwlock allocation failed.");
    }
  }

  return array;
}

void dmnsn_delete_array(dmnsn_array *array) {
  if (array) {
    if (pthread_rwlock_destroy(array->rwlock) != 0) {
      dmnsn_error(DMNSN_SEVERITY_LOW, "Leaking rwlock in array deallocation.");
    }

    free(array->rwlock);
    free(array->ptr);
    free(array);
  }
}

void
dmnsn_array_push(dmnsn_array *array, const void *obj)
{
  dmnsn_array_wrlock(array);
    dmnsn_array_set_impl(array, dmnsn_array_size_unlocked(array), obj);
  dmnsn_array_unlock(array);
}

void
dmnsn_array_pop(dmnsn_array *array, void *obj)
{
  size_t size;

  dmnsn_array_wrlock(array);
    size = dmnsn_array_size_unlocked(array);
    dmnsn_array_get_impl(array, size - 1, obj);
    dmnsn_array_resize_unlocked(array, size - 1);
  dmnsn_array_unlock(array);
}

void
dmnsn_array_get(const dmnsn_array *array, size_t i, void *obj)
{
  dmnsn_array_rdlock(array);
    dmnsn_array_get_impl(array, i, obj);
  dmnsn_array_unlock(array);
}

void
dmnsn_array_set(dmnsn_array *array, size_t i, const void *obj)
{
  dmnsn_array_wrlock(array);
    dmnsn_array_set_impl(array, i, obj);
  dmnsn_array_unlock(array);
}

size_t
dmnsn_array_size(const dmnsn_array *array)
{
  size_t size;

  dmnsn_array_rdlock(array);
    size = dmnsn_array_size_unlocked(array);
  dmnsn_array_unlock(array);

  return size;
}

void
dmnsn_array_resize(dmnsn_array *array, size_t length)
{
  dmnsn_array_wrlock(array);
    dmnsn_array_resize_unlocked(array, length);
  dmnsn_array_unlock(array);
}

void *
dmnsn_array_at(dmnsn_array *array, size_t i)
{
  if (i >= dmnsn_array_size_unlocked(array)) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Array index out of bounds.");
  }
  return (char *)array->ptr + array->obj_size*i;
}

size_t
dmnsn_array_size_unlocked(const dmnsn_array *array)
{
  return array->length;
}

void
dmnsn_array_resize_unlocked(dmnsn_array *array, size_t length)
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

void
dmnsn_array_rdlock(const dmnsn_array *array)
{
  if (pthread_rwlock_rdlock(array->rwlock) != 0) {
    /* Medium severity, because undefined behaviour is pretty likely if our
       reads and writes aren't synced */
    dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                "Couldn't acquire read-lock for array.");
  }
}

void
dmnsn_array_wrlock(dmnsn_array *array)
{
  if (pthread_rwlock_wrlock(array->rwlock) != 0) {
    /* Medium severity, because undefined behaviour is pretty likely if our
       reads and writes aren't synced */
    dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                "Couldn't acquire write-lock for array.");
  }
}

void
dmnsn_array_unlock(const dmnsn_array *array)
{
  if (pthread_rwlock_unlock(array->rwlock) != 0) {
    /* Medium severity, because if the array is locked, we're likely to hang
       the next time we try to read or write it */
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't unlock array.");
  }
}

static void
dmnsn_array_get_impl(const dmnsn_array *array, size_t i, void *obj)
{
  if (i >= dmnsn_array_size_unlocked(array)) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Array index out of bounds.");
  }
  memcpy(obj, array->ptr + array->obj_size*i, array->obj_size);
}

static void
dmnsn_array_set_impl(dmnsn_array *array, size_t i, const void *obj)
{
  if (i >= dmnsn_array_size_unlocked(array)) {
    dmnsn_array_resize_unlocked(array, i + 1);
  }
  memcpy(array->ptr + array->obj_size*i, obj, array->obj_size);
}
