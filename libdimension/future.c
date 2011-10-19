/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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

/* Allocate a new dmnsn_future* */
dmnsn_future *
dmnsn_new_future(void)
{
  dmnsn_future *future = dmnsn_malloc(sizeof(dmnsn_future));
  future->progress = 0;
  future->total    = 1;

  /* Initialize the rwlock, condition variable, and mutex */

  future->rwlock = dmnsn_malloc(sizeof(pthread_rwlock_t));
  dmnsn_initialize_rwlock(future->rwlock);

  future->cond = dmnsn_malloc(sizeof(pthread_cond_t));
  dmnsn_initialize_cond(future->cond);

  future->mutex = dmnsn_malloc(sizeof(pthread_mutex_t));
  dmnsn_initialize_mutex(future->mutex);

  future->min_wait = dmnsn_malloc(sizeof(double));
  *future->min_wait = 1.0;

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

    dmnsn_free(future->min_wait);

    dmnsn_destroy_mutex(future->mutex);
    dmnsn_free(future->mutex);

    dmnsn_destroy_cond(future->cond);
    dmnsn_free(future->cond);

    dmnsn_destroy_rwlock(future->rwlock);
    dmnsn_free(future->rwlock);

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

/* Get the current progress of the worker thread, in [0.0, 1.0] */
double
dmnsn_future_progress(const dmnsn_future *future)
{
  double progress;

  dmnsn_read_lock(future->rwlock);
    progress = (double)future->progress/future->total;
  dmnsn_unlock_rwlock(future->rwlock);

  return progress;
}

/* Wait until dmnsn_future_progress(future) >= progress */
void
dmnsn_future_wait(const dmnsn_future *future, double progress)
{
  dmnsn_lock_mutex(future->mutex);
    while (dmnsn_future_progress(future) < progress) {
      /* Set the minimum waited-on value */
      if (progress < *future->min_wait)
        *future->min_wait = progress;

      dmnsn_cond_wait(future->cond, future->mutex);
    }
  dmnsn_unlock_mutex(future->mutex);
}

/* Set the total number of loop iterations */
void
dmnsn_future_set_total(dmnsn_future *future, size_t total)
{
  dmnsn_write_lock(future->rwlock);
    future->total = total;
  dmnsn_unlock_rwlock(future->rwlock);
}

/* Increment the number of completed loop iterations */
void
dmnsn_future_increment(dmnsn_future *future)
{
  /* Allow a thread to be canceled whenever it increments a future object --
     this is close to PTHREAD_CANCEL_ASYNCHRONOUS but allows consistent state
     on cancellation */
  pthread_testcancel();

  dmnsn_write_lock(future->rwlock);
    ++future->progress;
  dmnsn_unlock_rwlock(future->rwlock);

  dmnsn_lock_mutex(future->mutex);
    if (dmnsn_future_progress(future) >= *future->min_wait) {
      *future->min_wait = 1.0;
      dmnsn_cond_broadcast(future->cond);
    }
  dmnsn_unlock_mutex(future->mutex);
}

/* Immediately set to 100% completion */
void
dmnsn_future_done(dmnsn_future *future)
{
  dmnsn_write_lock(future->rwlock);
    future->progress = future->total;
  dmnsn_unlock_rwlock(future->rwlock);

  dmnsn_lock_mutex(future->mutex);
    dmnsn_cond_broadcast(future->cond);
  dmnsn_unlock_mutex(future->mutex);
}
