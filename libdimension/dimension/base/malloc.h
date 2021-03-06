/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Dynamic memory.  dmnsn_malloc() and friends behave like their
 * non-dmnsn_-prefixed counterparts, but never return NULL.  If allocation
 * fails, they instead call dmnsn_error().
 */

#ifndef DMNSN_BASE_H
#error "Please include <dimension/base.h> instead of this header directly."
#endif

#include <stddef.h> /* For size_t */

/**
 * Allocate some memory.  Always use dmnsn_free() to free this memory, never
 * free().
 * @param[in] size  The size of the memory block to allocate.
 * @return The allocated memory area.
 */
void *dmnsn_malloc(size_t size);

/**
 * Allocate some memory.  Always use dmnsn_free() to free this memory, never
 * free().
 * @param[in] type  The type of the memory block to allocate.
 * @return The allocated memory area.
 */
#define DMNSN_MALLOC(type) ((type *)dmnsn_malloc(sizeof(type)))

/**
 * Expand or shrink an allocation created by dmnsn_malloc().
 * @param[in] ptr   The block to resize.
 * @param[in] size  The new size.
 * @return The resized memory area.
 */
void *dmnsn_realloc(void *ptr, size_t size);

/**
 * Duplicate a string.
 * @param[in] s  The string to duplicate.
 * @return A string with the same contents as \p s, suitable for release by
 *         dmnsn_free().
 */
char *dmnsn_strdup(const char *s);

/**
 * Free memory allocated by dmnsn_malloc() or dmnsn_strdup().
 * @param[in] ptr  The memory block to free, or NULL.
 */
void dmnsn_free(void *ptr);
