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

/*
 * Simple thread-safe generalized arrays, for returning variable-length arrays
 * from functions, and other fun stuff.
 */

#ifndef DIMENSION_ARRAY_H
#define DIMENSION_ARRAY_H

#include <pthread.h> /* For pthread_rwlock_t */
#include <stdlib.h>  /* For size_t */

typedef struct {
  void *ptr;
  size_t obj_size, length, capacity;

  /* Synchronicity control (pointer so it's not const) */
  pthread_rwlock_t *rwlock;
} dmnsn_array;

/* Array allocation never returns NULL - if dmnsn_new_array returns, it
   succeeded */
dmnsn_array *dmnsn_new_array(size_t obj_size);
void dmnsn_delete_array(dmnsn_array *array);

/* Thread-safe atomic array access */

void dmnsn_array_push(dmnsn_array *array, const void *obj);
void dmnsn_array_pop(dmnsn_array *array, void *obj);
void dmnsn_array_get(const dmnsn_array *array, size_t i, void *obj);
void dmnsn_array_set(dmnsn_array *array, size_t i, const void *obj);

size_t dmnsn_array_size(const dmnsn_array *array);
void   dmnsn_array_resize(dmnsn_array *array, size_t length);

/* Non-atomic operations for manual locking */
void *dmnsn_array_at(dmnsn_array *array, size_t i);
size_t dmnsn_array_size_unlocked(const dmnsn_array *array);
void dmnsn_array_resize_unlocked(dmnsn_array *array, size_t length);

/* Manual locking */
void dmnsn_array_rdlock(const dmnsn_array *array);
void dmnsn_array_wrlock(dmnsn_array *array);
void dmnsn_array_unlock(const dmnsn_array *array);

#endif /* DIMENSION_ARRAY_H */
