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
#include <stdatomic.h>

/// Number of pointers per block, we want a block to fit in a single page.
#define DMNSN_POOL_BLOCK_SIZE (4096/sizeof(void *) - 4)
/// Number of pointers per tidy block
#define DMNSN_TIDY_BLOCK_SIZE ((4096 - 4*sizeof(void *))/(sizeof(void *) + sizeof(dmnsn_callback_fn *)))

/// A single block in a thread-specific pool.
typedef struct dmnsn_pool_block {
  /// Current index into allocs[].
  size_t i;
  /// All allocations in the current block.
  void *allocs[DMNSN_POOL_BLOCK_SIZE];
  /// Tail pointer to the previous block in the global chain.
  struct dmnsn_pool_block *prev;
} dmnsn_pool_block;

/// A single tidy block in a thread-specific pool.
typedef struct dmnsn_tidy_block {
  /// Current index into allocs[].
  size_t i;
  /// All allocations in the current block.
  void *allocs[DMNSN_TIDY_BLOCK_SIZE];
  /// All cleanup callbacks in the current block.
  dmnsn_callback_fn *cleanup_fns[DMNSN_TIDY_BLOCK_SIZE];
  /// Tail pointer to the previous tidy block in the global chain.
  struct dmnsn_tidy_block *prev;
} dmnsn_tidy_block;

/// dmnsn_pool implementation.
struct dmnsn_pool {
  /// Thread-local regular block.
  pthread_key_t thread_block;
  /// Thread-local tidy block.
  pthread_key_t thread_tidy_block;

  /// Global chain of regular blocks.
  atomic(dmnsn_pool_block *) chain;
  /// Global chain of tidy blocks.
  atomic(dmnsn_tidy_block *) tidy_chain;
};

dmnsn_pool *
dmnsn_new_pool(void)
{
  dmnsn_pool *pool = DMNSN_MALLOC(dmnsn_pool);

  dmnsn_key_create(&pool->thread_block, NULL);
  dmnsn_key_create(&pool->thread_tidy_block, NULL);

  atomic_store_explicit(&pool->chain, NULL, memory_order_relaxed);
  atomic_store_explicit(&pool->tidy_chain, NULL, memory_order_relaxed);

  return pool;
}

void *
dmnsn_palloc(dmnsn_pool *pool, size_t size)
{
  dmnsn_pool_block *old_block = pthread_getspecific(pool->thread_block);

  dmnsn_pool_block *new_block = old_block;
  if (dmnsn_unlikely(!old_block || old_block->i == DMNSN_POOL_BLOCK_SIZE)) {
    new_block = DMNSN_MALLOC(dmnsn_pool_block);
    new_block->i = 0;
  }

  void *result = dmnsn_malloc(size);
  new_block->allocs[new_block->i++] = result;

  if (dmnsn_unlikely(new_block != old_block)) {
    dmnsn_setspecific(pool->thread_block, new_block);

    // Atomically update pool->chain
    dmnsn_pool_block *old_chain = atomic_exchange(&pool->chain, new_block);
    new_block->prev = old_chain;
  }

  return result;
}

void *
dmnsn_palloc_tidy(dmnsn_pool *pool, size_t size, dmnsn_callback_fn *cleanup_fn)
{
  dmnsn_assert(cleanup_fn != NULL, "NULL cleanup_fn");

  dmnsn_tidy_block *old_block = pthread_getspecific(pool->thread_tidy_block);

  dmnsn_tidy_block *new_block = old_block;
  if (dmnsn_unlikely(!old_block || old_block->i == DMNSN_TIDY_BLOCK_SIZE)) {
    new_block = DMNSN_MALLOC(dmnsn_tidy_block);
    new_block->i = 0;
  }

  void *result = dmnsn_malloc(size);

  size_t i = new_block->i;
  new_block->allocs[i] = result;
  new_block->cleanup_fns[i] = cleanup_fn;
  ++new_block->i;

  if (dmnsn_unlikely(new_block != old_block)) {
    dmnsn_setspecific(pool->thread_tidy_block, new_block);

    // Atomically update pool->tidy_chain
    dmnsn_tidy_block *old_chain = atomic_exchange(&pool->tidy_chain, new_block);
    new_block->prev = old_chain;
  }

  return result;
}

void
dmnsn_delete_pool(dmnsn_pool *pool)
{
  if (!pool) {
    return;
  }

  dmnsn_pool_block *block = atomic_load_explicit(&pool->chain, memory_order_relaxed);
  while (block) {
    // Free all the allocations
    for (size_t i = block->i; i-- > 0;) {
      dmnsn_free(block->allocs[i]);
    }

    // Free the block itself and go to the previous one
    dmnsn_pool_block *saved = block;
    block = block->prev;
    dmnsn_free(saved);
  }

  dmnsn_tidy_block *tidy_block = atomic_load_explicit(&pool->tidy_chain, memory_order_relaxed);
  while (tidy_block) {
    // Free all the allocations
    for (size_t i = tidy_block->i; i-- > 0;) {
      void *ptr = tidy_block->allocs[i];
      tidy_block->cleanup_fns[i](ptr);
      dmnsn_free(ptr);
    }

    // Free the block itself and go to the previous one
    dmnsn_tidy_block *saved = tidy_block;
    tidy_block = tidy_block->prev;
    dmnsn_free(saved);
  }

  dmnsn_key_delete(pool->thread_tidy_block);
  dmnsn_free(pool);
}
