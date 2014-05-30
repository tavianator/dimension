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
 * Memory pool implementation.
 */

#include "dimension-internal.h"

/** A single allocation and associated destructor. */
typedef struct dmnsn_allocation {
  void *ptr;
  dmnsn_callback_fn *callback;
} dmnsn_allocation;

/** Number of pointers per block, we want a block to fit in a single page. */
#define DMNSN_POOL_BLOCK_SIZE ((4096 - 4*sizeof(void *))/sizeof(dmnsn_allocation))

/** A single block in a thread pool. */
typedef struct dmnsn_pool_block {
  /** Current index into allocs[]. */
  size_t i;
  /** All allocations in the current block. */
  dmnsn_allocation allocs[DMNSN_POOL_BLOCK_SIZE];
  /** Tail pointer to the previous block in the global chain. */
  struct dmnsn_pool_block *prev;
} dmnsn_pool_block;

/** dmnsn_pool implementation. */
struct dmnsn_pool {
  /** Thread-local block. */
  pthread_key_t thread_block;

  /** Global chain of pools. */
  dmnsn_pool_block *chain;
  /** Mutex guarding the global chain. */
  pthread_mutex_t mutex;
};

dmnsn_pool *
dmnsn_new_pool(void)
{
  dmnsn_pool *pool = DMNSN_MALLOC(dmnsn_pool);
  dmnsn_key_create(&pool->thread_block, NULL);
  pool->chain = NULL;
  dmnsn_initialize_mutex(&pool->mutex);
  return pool;
}

void *
dmnsn_pool_alloc(dmnsn_pool *pool, size_t size, dmnsn_callback_fn *callback)
{
  dmnsn_pool_block *old_block = pthread_getspecific(pool->thread_block);

  dmnsn_pool_block *new_block = old_block;
  if (dmnsn_unlikely(!old_block || old_block->i == DMNSN_POOL_BLOCK_SIZE)) {
    new_block = DMNSN_MALLOC(dmnsn_pool_block);
    new_block->i = 0;
  }

  dmnsn_allocation *alloc = new_block->allocs + new_block->i;
  void *result = alloc->ptr = dmnsn_malloc(size);
  alloc->callback = callback;
  ++new_block->i;

  if (dmnsn_unlikely(new_block != old_block)) {
    dmnsn_setspecific(pool->thread_block, new_block);

    dmnsn_lock_mutex(&pool->mutex);
      new_block->prev = pool->chain;
      pool->chain = new_block;
    dmnsn_unlock_mutex(&pool->mutex);
  }

  return result;
}

void
dmnsn_delete_pool(dmnsn_pool *pool)
{
  if (!pool) {
    return;
  }

  dmnsn_pool_block *block = pool->chain;
  while (block) {
    /* Free all the allocations in reverse order */
    for (size_t i = block->i; i-- > 0;) {
      dmnsn_allocation *alloc = block->allocs + i;
      if (alloc->callback) {
        alloc->callback(alloc->ptr);
      }
      dmnsn_free(alloc->ptr);
    }

    /* Free the block itself and go to the previous one */
    dmnsn_pool_block *saved = block;
    block = block->prev;
    dmnsn_free(saved);
  }

  dmnsn_destroy_mutex(&pool->mutex);
  dmnsn_key_delete(pool->thread_block);
  dmnsn_free(pool);
}
