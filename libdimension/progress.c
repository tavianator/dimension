/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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

#include "dimension-impl.h"
#include <pthread.h>

/* A single element in an array for dmnsn_progress.  Progress of this item is
   progress/total. */
typedef struct {
  unsigned int progress, total;
} dmnsn_progress_element;

/* For thread synchronization */

static void dmnsn_progress_rdlock_impl(const dmnsn_progress *progress);
static void dmnsn_progress_wrlock_impl(dmnsn_progress *progress);
static void dmnsn_progress_unlock_impl(void *arg);

#define dmnsn_progress_rdlock(progress)                                 \
  dmnsn_progress_rdlock_impl(progress);                                 \
  pthread_cleanup_push(&dmnsn_progress_unlock_impl, (void *)progress);
#define dmnsn_progress_wrlock(progress)                                 \
  dmnsn_progress_wrlock_impl(progress);                                 \
  pthread_cleanup_push(&dmnsn_progress_unlock_impl, (void *)progress);
#define dmnsn_progress_unlock(progress)         \
  pthread_cleanup_pop(1);


/* Allocate a new dmnsn_progress* */
dmnsn_progress *
dmnsn_new_progress()
{
  dmnsn_progress *progress = dmnsn_malloc(sizeof(dmnsn_progress));
  progress->elements = dmnsn_new_array(sizeof(dmnsn_progress_element));

  dmnsn_progress_element element = { .progress = 0, .total = 1 };
  dmnsn_array_push(progress->elements, &element);

  /* Initialize the rwlock, condition variable, and mutex */

  progress->rwlock = dmnsn_malloc(sizeof(pthread_rwlock_t));
  if (pthread_rwlock_init(progress->rwlock, NULL) != 0) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't initialize read-write lock.");
  }

  progress->cond = dmnsn_malloc(sizeof(pthread_cond_t));
  if (pthread_cond_init(progress->cond, NULL) != 0) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't initialize condition variable.");
  }

  progress->mutex = dmnsn_malloc(sizeof(pthread_mutex_t));
  if (pthread_mutex_init(progress->mutex, NULL) != 0) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't initialize mutex.");
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
      /* Medium severity because an unjoined thread likely means that the thread
         is incomplete or invalid */
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Joining worker thread failed.");
    } else if (ptr) {
      retval = *(int *)ptr;
      dmnsn_free(ptr);
    }

    /* Free the progress object */
    if (pthread_rwlock_destroy(progress->rwlock) != 0) {
      dmnsn_error(DMNSN_SEVERITY_LOW, "Leaking rwlock.");
    }
    if (pthread_mutex_destroy(progress->mutex) != 0) {
      dmnsn_error(DMNSN_SEVERITY_LOW, "Leaking mutex.");
    }
    if (pthread_cond_destroy(progress->cond) != 0) {
      dmnsn_error(DMNSN_SEVERITY_LOW, "Leaking condition variable.");
    }
    dmnsn_free(progress->rwlock);
    dmnsn_free(progress->mutex);
    dmnsn_free(progress->cond);
    dmnsn_delete_array(progress->elements);
    dmnsn_free(progress);
  }

  return retval;
}

/* Get the current progress of the worker thread, in [0.0, 1.0] */
double
dmnsn_get_progress(const dmnsn_progress *progress)
{
  dmnsn_progress_element *element;
  double prog = 0.0;

  dmnsn_progress_rdlock(progress);
    size_t size = dmnsn_array_size(progress->elements);
    for (size_t i = 0; i < size; ++i) {
      element = dmnsn_array_at(progress->elements, size - i - 1);
      prog += element->progress;
      prog /= element->total;
    }
  dmnsn_progress_unlock(progress);

  return prog;
}

/* Wait until dmnsn_get_progress(progress) >= prog */
void
dmnsn_wait_progress(const dmnsn_progress *progress, double prog)
{
  if (pthread_mutex_lock(progress->mutex) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't lock condition mutex.");
    /* Busy-wait if we can't use the condition variable */
    while (dmnsn_get_progress(progress) < prog);
  } else {
    while (dmnsn_get_progress(progress) < prog) {
      /* Set the minimum waited-on value */
      if (prog < progress->min_wait)
        *progress->min_waitp = prog;

      if (pthread_cond_wait(progress->cond, progress->mutex) != 0) {
        dmnsn_error(DMNSN_SEVERITY_LOW,
                    "Couldn't wait on condition variable.");
      }
    }

    if (pthread_mutex_unlock(progress->mutex) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't unlock condition mutex.");
    }
  }
}

/* Start a new level of algorithmic nesting */
void
dmnsn_new_progress_element(dmnsn_progress *progress, unsigned int total)
{
  dmnsn_progress_element element = { .progress = 0, .total = total };
  dmnsn_progress_wrlock(progress);
    dmnsn_array_push(progress->elements, &element);
  dmnsn_progress_unlock(progress);
}

/* Only the innermost loop needs to call this function - it handles the rest
   upon loop completion (progress == total) */
void
dmnsn_increment_progress(dmnsn_progress *progress)
{
  dmnsn_progress_element *element;

  dmnsn_progress_wrlock(progress);
    size_t size = dmnsn_array_size(progress->elements);
    element = dmnsn_array_at(progress->elements, size - 1);
    ++element->progress; /* Increment the last element */

    while (element->progress >= element->total && size > 1) {
      /* As long as the last element is complete, pop it */
      --size;
      dmnsn_array_resize(progress->elements, size);
      element = dmnsn_array_at(progress->elements, size - 1);
      ++element->progress; /* Increment the next element */
    }
  dmnsn_progress_unlock(progress);

  if (pthread_mutex_lock(progress->mutex) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't lock condition mutex.");
  }

  if (dmnsn_get_progress(progress) >= progress->min_wait) {
    progress->min_wait = 1.0;

    if (pthread_cond_broadcast(progress->cond) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't signal condition variable.");
    }
  }

  if (pthread_mutex_unlock(progress->mutex) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't unlock condition mutex.");
  }
}

/* Immediately set to 100% completion */
void
dmnsn_done_progress(dmnsn_progress *progress)
{
  dmnsn_progress_element *element;

  dmnsn_progress_wrlock(progress);
    dmnsn_array_resize(progress->elements, 1);
    element = dmnsn_array_at(progress->elements, 0);
    element->progress = element->total;
  dmnsn_progress_unlock(progress);

  if (pthread_mutex_lock(progress->mutex) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't lock condition mutex.");
  }
  if (pthread_cond_broadcast(progress->cond) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't signal condition variable.");
  }
  if (pthread_mutex_unlock(progress->mutex) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't unlock condition mutex.");
  }
}

/* Thread synchronization */

static void
dmnsn_progress_rdlock_impl(const dmnsn_progress *progress)
{
  if (pthread_rwlock_rdlock(progress->rwlock) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't acquire read-lock.");
  }
}

static void
dmnsn_progress_wrlock_impl(dmnsn_progress *progress)
{
  if (pthread_rwlock_wrlock(progress->rwlock) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't acquire write-lock.");
  }
}

static void
dmnsn_progress_unlock_impl(void *arg)
{
  const dmnsn_progress *progress = arg;
  if (pthread_rwlock_unlock(progress->rwlock) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't unlock read-write lock.");
  }
}
