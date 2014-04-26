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
 * Future objects.
 */

#include "dimension-internal.h"
#include <pthread.h>

/**
 * Since C doesn't support anything like C++'s mutable, we fake it by casting
 * away the constness.  This is okay since all valid dmnsn_futures live on the
 * heap, so cannot be const.
 */
#define MUTATE(future) ((dmnsn_future *)(future))

/* Allocate a new dmnsn_future* */
dmnsn_future *
dmnsn_new_future(void)
{
  dmnsn_future *future = dmnsn_malloc(sizeof(dmnsn_future));
  future->progress = 0;
  future->total    = 1;

  dmnsn_initialize_mutex(&future->mutex);
  dmnsn_initialize_cond(&future->cond);

  future->min_wait = 1.0;

  future->nthreads = future->nrunning = 1;
  future->npaused = 0;
  dmnsn_initialize_cond(&future->none_running_cond);
  dmnsn_initialize_cond(&future->all_running_cond);
  dmnsn_initialize_cond(&future->resume_cond);

  return future;
}

/* Join the worker thread and delete `future'. */
int
dmnsn_future_join(dmnsn_future *future)
{
  void *ptr;
  int retval = -1;

  if (future) {
    /* Get the thread's return value */
    dmnsn_join_thread(future->thread, &ptr);
    if (ptr && ptr != PTHREAD_CANCELED) {
      retval = *(int *)ptr;
      dmnsn_free(ptr);
    }

    /* Free the future object */
    dmnsn_destroy_cond(&future->resume_cond);
    dmnsn_destroy_cond(&future->all_running_cond);
    dmnsn_destroy_cond(&future->none_running_cond);
    dmnsn_destroy_cond(&future->cond);
    dmnsn_destroy_mutex(&future->mutex);
    dmnsn_free(future);
  }

  return retval;
}

/* Cancel a background thread */
void
dmnsn_future_cancel(dmnsn_future *future)
{
  pthread_cancel(future->thread);
}

/**
 * Get the current progress, without locking anything.
 *
 * future->mutex must be locked for this call to be safe.
 */
static inline double
dmnsn_future_progress_unlocked(const dmnsn_future *future)
{
  return (double)future->progress/future->total;
}

/* Get the current progress of the worker thread, in [0.0, 1.0] */
double
dmnsn_future_progress(const dmnsn_future *future)
{
  dmnsn_future *mfuture = MUTATE(future);
  double progress;

  dmnsn_lock_mutex(&mfuture->mutex);
    progress = dmnsn_future_progress_unlocked(mfuture);
  dmnsn_unlock_mutex(&mfuture->mutex);

  return progress;
}

/* Find out whether the task is complete. */
bool
dmnsn_future_is_done(const dmnsn_future *future)
{
  dmnsn_future *mfuture = MUTATE(future);
  bool result;

  dmnsn_lock_mutex(&mfuture->mutex);
    result = future->progress == future->total;
  dmnsn_unlock_mutex(&mfuture->mutex);

  return result;
}

/* Wait until dmnsn_future_progress(future) >= progress */
void
dmnsn_future_wait(const dmnsn_future *future, double progress)
{
  dmnsn_future *mfuture = MUTATE(future);

  dmnsn_lock_mutex(&mfuture->mutex);
    while (dmnsn_future_progress_unlocked(mfuture) < progress) {
      /* Set the minimum waited-on value */
      if (progress < mfuture->min_wait) {
        mfuture->min_wait = progress;
      }

      dmnsn_cond_wait_safely(&mfuture->cond, &mfuture->mutex);
    }
  dmnsn_unlock_mutex(&mfuture->mutex);
}

/* Pause all threads working on a future. */
void
dmnsn_future_pause(dmnsn_future *future)
{
  dmnsn_lock_mutex(&future->mutex);
    while (future->nrunning < future->nthreads) {
      dmnsn_cond_wait_safely(&future->all_running_cond, &future->mutex);
    }
    if (future->npaused++ == 0) {
      while (future->nrunning > 0) {
        dmnsn_cond_wait_safely(&future->none_running_cond, &future->mutex);
      }
    }
  dmnsn_unlock_mutex(&future->mutex);
}

/* Resume all threads working on a future. */
void
dmnsn_future_resume(dmnsn_future *future)
{
  dmnsn_lock_mutex(&future->mutex);
    dmnsn_assert(future->npaused > 0,
                 "dmnsn_future_resume() without matching dmnsn_future_pause()");
    if (--future->npaused == 0) {
      dmnsn_cond_broadcast(&future->resume_cond);
    }
  dmnsn_unlock_mutex(&future->mutex);
}

/* Set the total number of loop iterations */
void
dmnsn_future_set_total(dmnsn_future *future, size_t total)
{
  dmnsn_lock_mutex(&future->mutex);
    future->total = total;
  dmnsn_unlock_mutex(&future->mutex);
}

static void
dmnsn_future_increment_cleanup(void *ptr)
{
  dmnsn_future *future = ptr;
  ++future->nrunning;
  dmnsn_unlock_mutex_impl(&future->mutex);
}

/* Increment the number of completed loop iterations */
void
dmnsn_future_increment(dmnsn_future *future)
{
  /* Allow a thread to be canceled whenever it increments a future object --
     this is close to PTHREAD_CANCEL_ASYNCHRONOUS but allows consistent state
     on cancellation */
  pthread_testcancel();

  dmnsn_lock_mutex(&future->mutex);
    ++future->progress;

    if (dmnsn_future_progress_unlocked(future) >= future->min_wait) {
      future->min_wait = 1.0;
      dmnsn_cond_broadcast(&future->cond);
    }

    if (future->npaused > 0) {
      dmnsn_assert(future->nrunning > 0, "More worker threads than expected");

      if (--future->nrunning == 0) {
        dmnsn_cond_broadcast(&future->none_running_cond);
      }

      pthread_cleanup_push(dmnsn_future_increment_cleanup, future);
        do {
          dmnsn_cond_wait(&future->resume_cond, &future->mutex);
        } while (future->npaused > 0);
      pthread_cleanup_pop(false);

      if (++future->nrunning == future->nthreads) {
        dmnsn_cond_broadcast(&future->all_running_cond);
      }
    }
  dmnsn_unlock_mutex(&future->mutex);
}

/* Immediately set to 100% completion */
void
dmnsn_future_finish(dmnsn_future *future)
{
  dmnsn_lock_mutex(&future->mutex);
    future->progress = future->total;
    future->nthreads = future->nrunning = 0;
    dmnsn_cond_broadcast(&future->cond);
    dmnsn_cond_broadcast(&future->none_running_cond);
    dmnsn_cond_broadcast(&future->all_running_cond);
  dmnsn_unlock_mutex(&future->mutex);
}

/* Set the number of threads */
void
dmnsn_future_set_nthreads(dmnsn_future *future, unsigned int nthreads)
{
  dmnsn_lock_mutex(&future->mutex);
    dmnsn_assert(future->nrunning == future->nthreads,
                 "dmnsn_future_set_nthreads() called with paused threads");
    future->nthreads = future->nrunning = nthreads;
  dmnsn_unlock_mutex(&future->mutex);
}

/* Notify completion of a worker thread */
void
dmnsn_future_finish_thread(dmnsn_future *future)
{
  dmnsn_lock_mutex(&future->mutex);
    dmnsn_assert(future->nthreads > 0,
                 "dmnsn_future_finish_thread() called with no threads");
    --future->nthreads;

    dmnsn_assert(future->nrunning > 0,
                 "dmnsn_future_finish_thread() called with no running threads");
    if (--future->nrunning == 0) {
      dmnsn_cond_broadcast(&future->none_running_cond);
    }
  dmnsn_unlock_mutex(&future->mutex);
}
