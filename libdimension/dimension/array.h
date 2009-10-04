/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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
 * Simple thread-safe generalized arrays, for returning variable-length arrays
 * from functions, and other fun stuff.  All functions are inline for
 * performance reasons.
 */

#ifndef DIMENSION_ARRAY_H
#define DIMENSION_ARRAY_H

#include <pthread.h> /* For pthread_rwlock_t */
#include <stdlib.h>  /* For size_t, malloc */
#include <string.h>  /* For memcpy */

typedef struct {
  void *ptr;
  size_t obj_size, length, capacity;
} dmnsn_array;

/* Array allocation never returns NULL - if dmnsn_new_array returns, it
   succeeded */
DMNSN_INLINE dmnsn_array *
dmnsn_new_array(size_t obj_size)
{
  dmnsn_array *array = (dmnsn_array *)malloc(sizeof(dmnsn_array));
  if (array) {
    array->obj_size = obj_size;
    array->length   = 0;
    array->capacity = 4; /* Start with capacity of 4 */

    /* Allocate the memory */
    array->ptr = malloc(array->capacity*array->obj_size);
    if (!array->ptr) {
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Array allocation failed.");
    }
  }

  return array;
}

/* Delete the array */
DMNSN_INLINE void
dmnsn_delete_array(dmnsn_array *array)
{
  if (array) {
    free(array->ptr);
    free(array);
  }
}

/* Get the size of the array */
DMNSN_INLINE size_t
dmnsn_array_size(const dmnsn_array *array)
{
  return array->length;
}

/* Set the size of the array */
DMNSN_INLINE void
dmnsn_array_resize(dmnsn_array *array, size_t length)
{
  if (length > array->capacity) {
    /* Resize if we don't have enough capacity */
    array->capacity = length*2; /* We are greedy */
    array->ptr = realloc(array->ptr, array->obj_size*array->capacity);
    if (!array->ptr) {
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Resizing array failed.");
    }
  }

  array->length = length;
}

/* Get the i'th object, bailing out if i is out of range */
DMNSN_INLINE void
dmnsn_array_get(const dmnsn_array *array, size_t i, void *obj)
{
  if (i >= dmnsn_array_size(array)) {
    /* Range check failed */
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Array index out of bounds.");
  }
  memcpy(obj, (char *)array->ptr + array->obj_size*i, array->obj_size);
}


/* Set the i'th object, expanding the array if necessary */
DMNSN_INLINE void
dmnsn_array_set(dmnsn_array *array, size_t i, const void *obj)
{
  if (i >= dmnsn_array_size(array)) {
    /* Resize if i is out of range */
    dmnsn_array_resize(array, i + 1);
  }
  memcpy((char *)array->ptr + array->obj_size*i, obj, array->obj_size);
}

/* Element access */
DMNSN_INLINE void *
dmnsn_array_at(dmnsn_array *array, size_t i)
{
  if (i >= dmnsn_array_size(array)) {
    /* Resize if i is out of range */
    dmnsn_array_resize(array, i + 1);
  }
  return (char *)array->ptr + array->obj_size*i;
}

/* Push obj to the end of the array */
DMNSN_INLINE void
dmnsn_array_push(dmnsn_array *array, const void *obj)
{
  dmnsn_array_set(array, dmnsn_array_size(array), obj);
}

/* Pop obj from the end of the array */
DMNSN_INLINE void
dmnsn_array_pop(dmnsn_array *array, void *obj)
{
  size_t size = dmnsn_array_size(array);
  dmnsn_array_get(array, size - 1, obj); /* Copy the object */
  dmnsn_array_resize(array, size - 1);   /* Shrink the array */
}

/* Insert an item into the middle of the array */
DMNSN_INLINE void
dmnsn_array_insert(dmnsn_array *array, size_t i, const void *obj)
{
  size_t size = dmnsn_array_size(array);
  /* Increase the size by 1 */
  dmnsn_array_resize(array, size + 1);
  /* Move the elements at and after `i' 1 to the right */
  memmove((char *)array->ptr + array->obj_size*(i + 1),
          (char *)array->ptr + array->obj_size*i,
          array->obj_size*(size - i));
  /* Insert `obj' at `i' */
  memcpy((char *)array->ptr + array->obj_size*i, obj, array->obj_size);
}

/* Remove an item from the middle of the array */
DMNSN_INLINE void
dmnsn_array_remove(dmnsn_array *array, size_t i)
{
  size_t size = dmnsn_array_size(array);
  /* Move the array elements after `i' 1 to the left */
  memmove((char *)array->ptr + array->obj_size*i,
          (char *)array->ptr + array->obj_size*(i + 1),
          array->obj_size*(size - i));
  /* Decrease the size by 1 */
  dmnsn_array_resize(array, size - 1);
}

#endif /* DIMENSION_ARRAY_H */
