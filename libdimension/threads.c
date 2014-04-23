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
 * Background threading.
 */

#include "dimension-internal.h"
#include <pthread.h>

/** The payload to pass to the pthread callback. */
typedef struct dmnsn_thread_payload {
  dmnsn_thread_fn *thread_fn;
  void *arg;
  dmnsn_future *future;
} dmnsn_thread_payload;

/** Clean up after a thread. */
static void
dmnsn_thread_cleanup(void *arg)
{
  dmnsn_thread_payload *payload = arg;
  dmnsn_future *future = payload->future;
  dmnsn_free(payload);

  dmnsn_future_done(future);
}

/** pthread callback -- call the real thread callback. */
static void *
dmnsn_thread(void *arg)
{
  dmnsn_thread_payload *payload = arg;
  int *ret;

  pthread_cleanup_push(dmnsn_thread_cleanup, payload);
    ret  = dmnsn_malloc(sizeof(int));
    *ret = payload->thread_fn(payload->arg);
  pthread_cleanup_pop(true);
  return ret;
}

void
dmnsn_new_thread(dmnsn_future *future, dmnsn_thread_fn *thread_fn, void *arg)
{
  dmnsn_thread_payload *payload = dmnsn_malloc(sizeof(dmnsn_thread_payload));
  payload->thread_fn = thread_fn;
  payload->arg       = arg;
  payload->future    = future;

  if (pthread_create(&future->thread, NULL, dmnsn_thread, payload) != 0) {
    dmnsn_error("Couldn't start thread.");
  }
}

/** Payload for threads executed by dmnsn_execute_concurrently(). */
typedef struct dmnsn_ccthread_payload {
  dmnsn_future *future;
  dmnsn_ccthread_fn *ccthread_fn;
  void *arg;
  unsigned int thread, nthreads;
  int ret;
  bool running;
} dmnsn_ccthread_payload;

static void *
dmnsn_concurrent_thread(void *ptr)
{
  dmnsn_ccthread_payload *payload = ptr;
  payload->ret = payload->ccthread_fn(payload->arg, payload->thread,
                                      payload->nthreads);
  if (payload->future) {
    dmnsn_future_thread_done(payload->future);
  }
  return NULL;
}

typedef struct dmnsn_ccthread_cleanup_payload {
  dmnsn_future *future;
  pthread_t *threads;
  dmnsn_ccthread_payload *payloads;
  unsigned int nthreads;
} dmnsn_ccthread_cleanup_payload;

static void
dmnsn_ccthread_cleanup(void *ptr)
{
  dmnsn_ccthread_cleanup_payload *payload = ptr;

  for (unsigned int i = 0; i < payload->nthreads; ++i) {
    if (payload->payloads[i].running) {
      pthread_cancel(payload->threads[i]);
    }
  }

  for (unsigned int i = 0; i < payload->nthreads; ++i) {
    if (payload->payloads[i].running) {
      dmnsn_join_thread(payload->threads[i], NULL);
    }
  }

  if (payload->future) {
    dmnsn_future_set_nthreads(payload->future, 1);
  }
}

int
dmnsn_execute_concurrently(dmnsn_future *future, dmnsn_ccthread_fn *ccthread_fn,
                           void *arg, unsigned int nthreads)
{
  dmnsn_assert(nthreads > 0, "Attempt to execute using 0 concurrent threads.");

  if (future) {
    dmnsn_future_set_nthreads(future, nthreads);
  }

  pthread_t threads[nthreads];
  dmnsn_ccthread_payload payloads[nthreads];
  for (unsigned int i = 0; i < nthreads; ++i) {
    payloads[i].running = false;
  }

  int ret = 0;
  dmnsn_ccthread_cleanup_payload cleanup_payload = {
    .future = future,
    .threads = threads,
    .payloads = payloads,
    .nthreads = nthreads,
  };
  pthread_cleanup_push(dmnsn_ccthread_cleanup, &cleanup_payload);
    for (unsigned int i = 0; i < nthreads; ++i) {
      payloads[i].future      = future;
      payloads[i].ccthread_fn = ccthread_fn;
      payloads[i].arg         = arg;
      payloads[i].thread      = i;
      payloads[i].nthreads    = nthreads;
      payloads[i].ret         = -1;
      if (pthread_create(&threads[i], NULL, dmnsn_concurrent_thread,
                         &payloads[i]) != 0)
      {
        dmnsn_error("Couldn't start worker thread.");
      }
      payloads[i].running = true;
    }

    for (unsigned int i = 0; i < nthreads; ++i) {
      dmnsn_join_thread(threads[i], NULL);
      payloads[i].running = false;
      if (payloads[i].ret != 0) {
        ret = payloads[i].ret;
      }
    }
  pthread_cleanup_pop(false);

  if (future) {
    dmnsn_future_set_nthreads(future, 1);
  }

  return ret;
}

/* pthread wrappers */

void
dmnsn_initialize_mutex(pthread_mutex_t *mutex)
{
  if (pthread_mutex_init(mutex, NULL) != 0) {
    dmnsn_error("Couldn't initialize mutex.");
  }
}

void
dmnsn_lock_mutex_impl(pthread_mutex_t *mutex)
{
  if (pthread_mutex_lock(mutex) != 0) {
    dmnsn_error("Couldn't lock mutex.");
  }
}

void
dmnsn_unlock_mutex_impl(void *mutex)
{
  if (pthread_mutex_unlock(mutex) != 0) {
    dmnsn_error("Couldn't unlock mutex.");
  }
}

void
dmnsn_destroy_mutex(pthread_mutex_t *mutex)
{
  if (pthread_mutex_destroy(mutex) != 0) {
    dmnsn_warning("Couldn't destroy mutex.");
  }
}

void
dmnsn_initialize_rwlock(pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock_init(rwlock, NULL) != 0) {
    dmnsn_error("Couldn't initialize read-write lock.");
  }
}

void
dmnsn_read_lock_impl(pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock_rdlock(rwlock) != 0) {
    dmnsn_error("Couldn't acquire read lock.");
  }
}

void
dmnsn_write_lock_impl(pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock_wrlock(rwlock) != 0) {
    dmnsn_error("Couldn't acquire write lock.");
  }
}

void
dmnsn_unlock_rwlock_impl(pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock_unlock(rwlock) != 0) {
    dmnsn_error("Couldn't unlock read-write lock.");
  }
}

void
dmnsn_destroy_rwlock(pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock_destroy(rwlock) != 0) {
    dmnsn_warning("Couldn't destroy read-write lock.");
  }
}

void
dmnsn_initialize_cond(pthread_cond_t *cond)
{
  if (pthread_cond_init(cond, NULL) != 0) {
    dmnsn_error("Couldn't initialize condition variable.");
  }
}

void
dmnsn_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  if (pthread_cond_wait(cond, mutex) != 0) {
    dmnsn_error("Couldn't wait on condition variable.");
  }
}

void
dmnsn_cond_broadcast(pthread_cond_t *cond)
{
  if (pthread_cond_broadcast(cond) != 0) {
    dmnsn_error("Couldn't signal condition variable.");
  }
}

void
dmnsn_destroy_cond(pthread_cond_t *cond)
{
  if (pthread_cond_destroy(cond) != 0) {
    dmnsn_warning("Couldn't destroy condition variable.");
  }
}

void
dmnsn_once(pthread_once_t *once, dmnsn_once_fn *once_fn)
{
  if (pthread_once(once, once_fn) != 0) {
    dmnsn_error("Couldn't call one-shot function.");
  }
}

void
dmnsn_key_create(pthread_key_t *key, dmnsn_callback_fn *destructor)
{
  if (pthread_key_create(key, destructor) != 0) {
    dmnsn_error("Couldn't initialize thread-specific pointer.");
  }
}

void
dmnsn_setspecific(pthread_key_t key, const void *value)
{
  if (pthread_setspecific(key, value) != 0) {
    dmnsn_error("Couldn't set thread-specific pointer.");
  }
}

void
dmnsn_key_delete(pthread_key_t key)
{
  if (pthread_key_delete(key) != 0) {
    dmnsn_warning("Couldn't destroy thread-specific pointer.");
  }
}

void
dmnsn_join_thread(pthread_t thread, void **retval)
{
  if (pthread_join(thread, retval) != 0) {
    dmnsn_error("Couldn't join thread.");
  }
}
