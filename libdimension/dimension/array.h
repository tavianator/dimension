/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Simple dynamic arrays.
 */

#include <stddef.h> /* For size_t */
#include <stdlib.h> /* For qsort() */
#include <string.h> /* For memcpy() */

/** Dynamic array type. */
typedef struct dmnsn_array {
  void *ptr;       /**< @internal The actual memory. */
  size_t obj_size; /**< @internal The size of each object. */
  size_t length;   /**< @internal The current size of the array. */
  size_t capacity; /**< @internal The size of the allocated space. */
} dmnsn_array;

/**
 * @internal
 * Initialize a new array.
 * @param[out] array  The array to initialize.
 * @param[in] obj_size  The size of the objects to store in the array.
 */
DMNSN_INLINE void
dmnsn_init_array(dmnsn_array *array, size_t obj_size)
{
  array->obj_size = obj_size;
  array->length   = 0;
  array->capacity = 2; /* Start with capacity of 2 */

  /* Allocate the memory */
  array->ptr = dmnsn_malloc(array->capacity*array->obj_size);
}

/**
 * Allocate an array.
 * @param[in] obj_size  The size of the objects to store in the array.
 * @return An empty array.
 */
DMNSN_INLINE dmnsn_array *
dmnsn_new_array(size_t obj_size)
{
  dmnsn_array *array = DMNSN_MALLOC(dmnsn_array);
  dmnsn_init_array(array, obj_size);
  return array;
}

/**
 * Allocate an array.
 * @param[in] type  Type type of element to store in the array.
 * @return An empty array.
 */
#define DMNSN_NEW_ARRAY(type) (dmnsn_new_array(sizeof(type)))

/**
 * Delete an array.
 * @param[in,out] array  The array to delete.
 */
DMNSN_INLINE void
dmnsn_delete_array(dmnsn_array *array)
{
  if (array) {
    dmnsn_free(array->ptr);
    dmnsn_free(array);
  }
}

/**
 * @internal
 * Free a pool-allocated array.
 * @param[in,out] ptr  The array to clean up.
 */
void dmnsn_array_cleanup(void *ptr);

/**
 * Allocate an array from a pool.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] obj_size  The size of the objects to store in the array.
 * @return An empty array.
 */
DMNSN_INLINE dmnsn_array *
dmnsn_palloc_array(dmnsn_pool *pool, size_t obj_size)
{
  dmnsn_array *array = DMNSN_PALLOC_TIDY(pool, dmnsn_array, dmnsn_array_cleanup);
  dmnsn_init_array(array, obj_size);
  return array;
}

/**
 * Allocate an array from a pool.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] type  Type type of element to store in the array.
 * @return An empty array.
 */
#define DMNSN_PALLOC_ARRAY(pool, type) (dmnsn_palloc_array(pool, sizeof(type)))

/**
 * Get the size of the array.
 * @param[in] array  The array in question.
 * @return The number of elements in the array.
 */
DMNSN_INLINE size_t
dmnsn_array_size(const dmnsn_array *array)
{
  return array->length;
}

/**
 * Set the size of the array.
 * @param[in,out] array   The array to resize.
 * @param[in]     length  The new length of the array.
 */
DMNSN_INLINE void
dmnsn_array_resize(dmnsn_array *array, size_t length)
{
  if (length > array->capacity) {
    /* Resize if we don't have enough capacity */
    array->capacity = length*2; /* We are greedy */
    array->ptr = dmnsn_realloc(array->ptr, array->obj_size*array->capacity);
  }

  array->length = length;
}

/**
 * Copy an array.
 * @param[in] array  The array to copy.
 * @return A copy of the array.
 */
DMNSN_INLINE dmnsn_array *
dmnsn_array_copy(const dmnsn_array *array)
{
  dmnsn_array *copy = dmnsn_new_array(array->obj_size);
  dmnsn_array_resize(copy, dmnsn_array_size(array));
  memcpy(copy->ptr, array->ptr, dmnsn_array_size(array)*array->obj_size);
  return copy;
}

/**
 * Split an array in half.
 * @param[in,out] array  The array to split.
 * @return The second half of the array.
 */
DMNSN_INLINE dmnsn_array *
dmnsn_array_split(dmnsn_array *array)
{
  dmnsn_array *half = dmnsn_new_array(array->obj_size);
  size_t new_size = dmnsn_array_size(array)/2;
  size_t old_size = dmnsn_array_size(array) - new_size;
  dmnsn_array_resize(half, new_size);
  memcpy(half->ptr, (char *)array->ptr + old_size*array->obj_size,
         new_size*array->obj_size);
  dmnsn_array_resize(array, old_size);
  return half;
}

/**
 * Get the i'th element.
 * @param[in]  array  The array to access.
 * @param[in]  i      The index of the element to extract.
 * @param[out] obj    The location to store the extracted object.
 */
DMNSN_INLINE void
dmnsn_array_get(const dmnsn_array *array, size_t i, void *obj)
{
  dmnsn_assert(i < dmnsn_array_size(array), "Array index out of bounds.");
  memcpy(obj, (char *)array->ptr + array->obj_size*i, array->obj_size);
}

/**
 * Set the i'th object, expanding the array if necessary.
 * @param[in,out] array  The array to modify.
 * @param[in]     i      The index to update.
 * @param[in]     obj    The location of the object to copy into the array.
 */
DMNSN_INLINE void
dmnsn_array_set(dmnsn_array *array, size_t i, const void *obj)
{
  if (i >= dmnsn_array_size(array)) {
    /* Resize if i is out of range */
    dmnsn_array_resize(array, i + 1);
  }
  memcpy((char *)array->ptr + array->obj_size*i, obj, array->obj_size);
}

/**
 * First element.
 * @param[in] array  The array to index.
 * @return The address of the first element of the array.
 */
DMNSN_INLINE void *
dmnsn_array_first(const dmnsn_array *array)
{
  return array->ptr;
}

/**
 * Last element.
 * @param[in] array  The array to index.
 * @return The address of the last element of the array.
 */
DMNSN_INLINE void *
dmnsn_array_last(const dmnsn_array *array)
{
  return (char *)array->ptr + array->obj_size*(array->length - 1);
}

/**
 * Arbitrary element access.
 * @param[in] array  The array to index.
 * @param[in] i      The index to access.
 * @return The address of the i'th element of the array.
 */
DMNSN_INLINE void *
dmnsn_array_at(const dmnsn_array *array, size_t i)
{
  dmnsn_assert(i < dmnsn_array_size(array), "Array index out of bounds.");
  return (char *)array->ptr + array->obj_size*i;
}

/**
 * Push an object to the end of the array.
 * @param[in,out] array  The array to append to.
 * @param[in]     obj    The location of the object to push.
 */
DMNSN_INLINE void
dmnsn_array_push(dmnsn_array *array, const void *obj)
{
  dmnsn_array_set(array, dmnsn_array_size(array), obj);
}

/**
 * Pop an object from the end of the array.
 * @param[in,out] array  The array to pop from.
 * @param[out]    obj    The location to store the extracted object.
 */
DMNSN_INLINE void
dmnsn_array_pop(dmnsn_array *array, void *obj)
{
  size_t size = dmnsn_array_size(array);
  dmnsn_assert(size > 0, "Array is empty.");
  dmnsn_array_get(array, size - 1, obj); /* Copy the object */
  dmnsn_array_resize(array, size - 1);   /* Shrink the array */
}

/**
 * Insert an item into the middle of the array.  This is O(n).
 * @param[in,out] array  The array to insert to.
 * @param[in]     i      The index at which to insert.
 * @param[in]     obj    The object to insert.
 */
DMNSN_INLINE void
dmnsn_array_insert(dmnsn_array *array, size_t i, const void *obj)
{
  size_t size = dmnsn_array_size(array) + 1;
  if (i >= size)
    size = i + 1;
  dmnsn_array_resize(array, size);

  /* Move the elements at and after `i' 1 to the right */
  memmove((char *)array->ptr + array->obj_size*(i + 1),
          (char *)array->ptr + array->obj_size*i,
          array->obj_size*(size - i - 1));
  /* Insert `obj' at `i' */
  memcpy((char *)array->ptr + array->obj_size*i, obj, array->obj_size);
}

/**
 * Remove an item from the middle of the array.  This is O(n).
 * @param[in,out] array  The array to remove from.
 * @param[in]     i      The index to remove.
 */
DMNSN_INLINE void
dmnsn_array_remove(dmnsn_array *array, size_t i)
{
  size_t size = dmnsn_array_size(array);
  dmnsn_assert(i < size, "Array index out of bounds.");
  /* Move the array elements after `i' 1 to the left */
  memmove((char *)array->ptr + array->obj_size*i,
          (char *)array->ptr + array->obj_size*(i + 1),
          array->obj_size*(size - i - 1));
  /* Decrease the size by 1 */
  dmnsn_array_resize(array, size - 1);
}

/**
 * Apply a callback to each element of an array.
 * @param[in,out] array     The array.
 * @param[in]     callback  The callback to apply to the elements.
 */
DMNSN_INLINE void
dmnsn_array_apply(dmnsn_array *array, dmnsn_callback_fn *callback)
{
  char *i, *last = (char *)dmnsn_array_last(array);
  for (i = (char *)dmnsn_array_first(array); i <= last; i += array->obj_size) {
    callback((void *)i);
  }
}

/**
 * Callback type for array sorting.
 * @param[in] a  The first element.
 * @param[in] b  The second element.
 * @return A negative value iff a < b, zero iff a == b, and a positive value iff
 *         a > b.
 */
typedef int dmnsn_array_comparator_fn(const void *a, const void *b);

/**
 * Sort an array.
 * @param[in,out] array       The array to sort.
 * @param[in]     comparator  The sorting comparator to use.
 */
DMNSN_INLINE void
dmnsn_array_sort(dmnsn_array *array, dmnsn_array_comparator_fn *comparator)
{
  qsort(array->ptr, dmnsn_array_size(array), array->obj_size, comparator);
}

/* Macros to shorten array iteration */

/**
 * Iterate over an array.  For example,
 * @code
 * DMNSN_ARRAY_FOREACH (int *, i, array) {
 *   printf("%d\n", *i);
 * }
 * @endcode
 *
 * @param     type   The (pointer) type to use as an iterator.
 * @param     i      The name of the iterator within the loop body.
 * @param[in] array  The array to loop over.
 */
#define DMNSN_ARRAY_FOREACH(type, i, array)                                    \
  for (type i = dmnsn_array_first(array);                                      \
       (size_t)(i - (type)dmnsn_array_first(array)) < dmnsn_array_size(array); \
       ++i)
