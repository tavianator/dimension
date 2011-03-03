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

/** Read-lock a progress object. */
static void dmnsn_progress_rdlock(const dmnsn_progress *progress);
/** Write-lock a progress object. */
static void dmnsn_progress_wrlock(dmnsn_progress *progress);
/** Unlock a progress object. */
static void dmnsn_progress_unlock(const dmnsn_progress *progress);

/* Allocate a new dmnsn_progress* */
dmnsn_progress *
dmnsn_new_progress(void)
{
  dmnsn_progress *progress = dmnsn_malloc(sizeof(dmnsn_progress));
  progress->progress = 0;
  progress->total    = 1;

  /* Initialize the rwlock, condition variable, and mutex */

  progress->rwlock = dmnsn_malloc(sizeof(pthread_rwlock_t));
  if (pthread_rwlock_init(progress->rwlock, NULL) != 0) {
    dmnsn_error("Couldn't initialize read-write lock.");
  }

  progress->cond = dmnsn_malloc(sizeof(pthread_cond_t));
  if (pthread_cond_init(progress->cond, NULL) != 0) {
    dmnsn_error("Couldn't initialize condition variable.");
  }

  progress->mutex = dmnsn_malloc(sizeof(pthread_mutex_t));
  if (pthread_mutex_init(progress->mutex, NULL) != 0) {
    dmnsn_error("Couldn't initialize mutex.");
  }

  progress->min_wait = 1.0;
  progress->min_waitp = &progress->min_wait;

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
    if (pthread_rwlock_destroy(progress->rwlock) != 0) {
      dmnsn_warning("Leaking rwlock.");
    }
    if (pthread_mutex_destroy(progress->mutex) != 0) {
      dmnsn_warning("Leaking mutex.");
    }
    if (pthread_cond_destroy(progress->cond) != 0) {
      dmnsn_warning("Leaking condition variable.");
    }
    dmnsn_free(progress->rwlock);
    dmnsn_free(progress->mutex);
    dmnsn_free(progress->cond);
    dmnsn_free(progress);
  }

  return retval;
}

/* Get the current progress of the worker thread, in [0.0, 1.0] */
double
dmnsn_get_progress(const dmnsn_progress *progress)
{
  double prog;

  dmnsn_progress_rdlock(progress);
    prog = (double)progress->progress/progress->total;
  dmnsn_progress_unlock(progress);

  return prog;
}

/* Wait until dmnsn_get_progress(progress) >= prog */
void
dmnsn_wait_progress(const dmnsn_progress *progress, double prog)
{
  if (pthread_mutex_lock(progress->mutex) == 0) {
    while (dmnsn_get_progress(progress) < prog) {
      /* Set the minimum waited-on value */
      if (prog < progress->min_wait)
        *progress->min_waitp = prog;

      if (pthread_cond_wait(progress->cond, progress->mutex) != 0) {
        dmnsn_error("Couldn't wait on condition variable.");
      }
    }

    if (pthread_mutex_unlock(progress->mutex) != 0) {
      dmnsn_error("Couldn't unlock condition mutex.");
    }
  } else {
    dmnsn_error("Couldn't lock condition mutex.");
  }
}

/* Set the total number of loop iterations */
void
dmnsn_set_progress_total(dmnsn_progress *progress, size_t total)
{
  dmnsn_progress_wrlock(progress);
    progress->total = total;
  dmnsn_progress_unlock(progress);
}

/* Increment the number of completed loop iterations */
void
dmnsn_increment_progress(dmnsn_progress *progress)
{
  dmnsn_progress_wrlock(progress);
    ++progress->progress;
  dmnsn_progress_unlock(progress);

  if (pthread_mutex_lock(progress->mutex) != 0) {
    dmnsn_error("Couldn't lock condition mutex.");
  }

  if (dmnsn_get_progress(progress) >= progress->min_wait) {
    progress->min_wait = 1.0;

    if (pthread_cond_broadcast(progress->cond) != 0) {
      dmnsn_error("Couldn't signal condition variable.");
    }
  }

  if (pthread_mutex_unlock(progress->mutex) != 0) {
    dmnsn_error("Couldn't unlock condition mutex.");
  }
}

/* Immediately set to 100% completion */
void
dmnsn_done_progress(dmnsn_progress *progress)
{
  dmnsn_progress_wrlock(progress);
    progress->progress = progress->total;
  dmnsn_progress_unlock(progress);

  if (pthread_mutex_lock(progress->mutex) != 0) {
    dmnsn_error("Couldn't lock condition mutex.");
  }
  if (pthread_cond_broadcast(progress->cond) != 0) {
    dmnsn_error("Couldn't signal condition variable.");
  }
  if (pthread_mutex_unlock(progress->mutex) != 0) {
    dmnsn_error("Couldn't unlock condition mutex.");
  }
}

/* Thread synchronization */

static void
dmnsn_progress_rdlock(const dmnsn_progress *progress)
{
  if (pthread_rwlock_rdlock(progress->rwlock) != 0) {
    dmnsn_error("Couldn't acquire read-lock.");
  }
}

static void
dmnsn_progress_wrlock(dmnsn_progress *progress)
{
  if (pthread_rwlock_wrlock(progress->rwlock) != 0) {
    dmnsn_error("Couldn't acquire write-lock.");
  }
}

static void
dmnsn_progress_unlock(const dmnsn_progress *progress)
{
  if (pthread_rwlock_unlock(progress->rwlock) != 0) {
    dmnsn_error("Couldn't unlock read-write lock.");
  }
}
