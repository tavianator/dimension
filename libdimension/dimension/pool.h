/*************************************************************************
 * Copyright (C) 2014 Tavian Barnes <tavianator@tavianator.com>          *
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
 * Memory pools.  Rather than more complicated garbage collection methods like
 * reference counting, objects are allocated out of pools which are freed all at
 * once once a scene is rendered (for example).
 */

#include <stddef.h> // For size_t

// Forward-declare dmnsn_pool.
typedef struct dmnsn_pool dmnsn_pool;

/**
 * Create a new memory pool.
 * @return The new pool.
 */
dmnsn_pool *dmnsn_new_pool(void);

/**
 * Allocate some memory from a pool.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] size  The size of the memory block to allocate.
 * @return The allocated memory area.
 */
void *dmnsn_palloc(dmnsn_pool *pool, size_t size);

/**
 * Allocate some memory from a pool.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] size  The size of the memory block to allocate.
 * @param[in] cleanup_fn  A callback to invoke before the memory is freed.
 * @return The allocated memory area.
 */
void *dmnsn_palloc_tidy(dmnsn_pool *pool, size_t size, dmnsn_callback_fn *cleanup_fn);

/**
 * Allocate some memory from a pool.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] type  The type of the memory block to allocate.
 * @return The allocated memory area.
 */
#define DMNSN_PALLOC(pool, type) ((type *)dmnsn_palloc((pool), sizeof(type)))

/**
 * Allocate some memory from a pool.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] type  The type of the memory block to allocate.
 * @param[in] cleanup_fn  A callback to invoke before the memory is freed.
 * @return The allocated memory area.
 */
#define DMNSN_PALLOC_TIDY(pool, type, cleanup_fn) ((type *)dmnsn_palloc_tidy((pool), sizeof(type), (cleanup_fn)))

/**
 * Free a memory pool and all associated allocations.
 * @param[in] pool  The memory pool to free.
 */
void dmnsn_delete_pool(dmnsn_pool *pool);
