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

#ifndef DIMENSION_ARRAY_H
#define DIMENSION_ARRAY_H

/*
 * Simple generalized arrays, for returning variable-length arrays from
 * functions, and other fun stuff.
 */

#include <stdlib.h> /* For size_t */

typedef struct {
  void *ptr;
  size_t obj_size, length, capacity;
} dmnsn_array;

dmnsn_array *dmnsn_new_array(size_t obj_size);

void dmnsn_array_push(dmnsn_array *array, const void *obj);
void dmnsn_array_pop(dmnsn_array *array, void *obj);

void dmnsn_array_get(const dmnsn_array *array, size_t i, void *obj);
void dmnsn_array_set(dmnsn_array *array, size_t i, const void *obj);

void dmnsn_array_resize(dmnsn_array *array, size_t length);

void dmnsn_delete_array(dmnsn_array *array);

#endif /* DIMENSION_ARRAY_H */
