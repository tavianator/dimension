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
 * Progress objects.
 */

#include "dimension-impl.h"
#include <pthread.h>

/* Allocate a new dmnsn_progress* */
dmnsn_progress *
dmnsn_new_progress(void)
{
  dmnsn_progress *progress = dmnsn_malloc(sizeof(dmnsn_progress));
  progress->progress = 0;
  progress->total    = 1;

  /* Initialize the rwlock, condition variable, and mutex */

  progress->rwlock = dmnsn_malloc(sizeof(pthread_rwlock_t));
  dmnsn_initialize_rwlock(progress->rwlock);

  progress->cond = dmnsn_malloc(sizeof(pthread_cond_t));
  dmnsn_initialize_cond(progress->cond);

  progress->mutex = dmnsn_malloc(sizeof(pthread_mutex_t));
  dmnsn_initialize_mutex(progress->mutex);

  progress->min_wait = dmnsn_malloc(sizeof(double));
  *progress->min_wait = 1.0;

  return progress;
}

/* Join the worker thread and delete `progress'. */
int
dmnsn_finish_progress(dmnsn_progress *progress)
{
  void *ptr;
  int retval = -1;

  if (progress) {
    /* Get the thread's return value */
    if (pthread_join(progress->thread, &ptr) != 0) {
      dmnsn_error("Joining worker thread failed.");
    } else if (ptr) {
      retval = *(int *)ptr;
      dmnsn_free(ptr);
    }

    /* Free the progress object */

    dmnsn_free(progress->min_wait);

    dmnsn_destroy_mutex(progress->mutex);
    dmnsn_free(progress->mutex);

    dmnsn_destroy_cond(progress->cond);
    dmnsn_free(progress->cond);

    dmnsn_destroy_rwlock(progress->rwlock);
    dmnsn_free(progress->rwlock);

    dmnsn_free(progress);
  }

  return retval;
}

/* Get the current progress of the worker thread, in [0.0, 1.0] */
double
dmnsn_get_progress(const dmnsn_progress *progress)
{
  double prog;

  dmnsn_read_lock(progress->rwlock);
    prog = (double)progress->progress/progress->total;
  dmnsn_unlock_rwlock(progress->rwlock);

  return prog;
}

/* Wait until dmnsn_get_progress(progress) >= prog */
void
dmnsn_wait_progress(const dmnsn_progress *progress, double prog)
{
  dmnsn_lock_mutex(progress->mutex);
    while (dmnsn_get_progress(progress) < prog) {
      /* Set the minimum waited-on value */
      if (prog < *progress->min_wait)
        *progress->min_wait = prog;

      dmnsn_cond_wait(progress->cond, progress->mutex);
    }
  dmnsn_unlock_mutex(progress->mutex);
}

/* Set the total number of loop iterations */
void
dmnsn_set_progress_total(dmnsn_progress *progress, size_t total)
{
  dmnsn_write_lock(progress->rwlock);
    progress->total = total;
  dmnsn_unlock_rwlock(progress->rwlock);
}

/* Increment the number of completed loop iterations */
void
dmnsn_increment_progress(dmnsn_progress *progress)
{
  dmnsn_write_lock(progress->rwlock);
    ++progress->progress;
  dmnsn_unlock_rwlock(progress->rwlock);

  dmnsn_lock_mutex(progress->mutex);
    if (dmnsn_get_progress(progress) >= *progress->min_wait) {
      *progress->min_wait = 1.0;

      dmnsn_cond_broadcast(progress->cond);
    }
  dmnsn_unlock_mutex(progress->mutex);
}

/* Immediately set to 100% completion */
void
dmnsn_done_progress(dmnsn_progress *progress)
{
  dmnsn_write_lock(progress->rwlock);
    progress->progress = progress->total;
  dmnsn_unlock_rwlock(progress->rwlock);

  dmnsn_lock_mutex(progress->mutex);
    dmnsn_cond_broadcast(progress->cond);
  dmnsn_unlock_mutex(progress->mutex);
}
