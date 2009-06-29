/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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

#include "dimension.h"
#include <pthread.h>
#include <stdlib.h> /* For malloc */

/* For thread synchronization */
static void dmnsn_progress_rdlock(const dmnsn_progress *progress);
static void dmnsn_progress_wrlock(dmnsn_progress *progress);
static void dmnsn_progress_unlock(const dmnsn_progress *progress);

/* Allocate a new dmnsn_progress*, returning NULL on failure */
dmnsn_progress *
dmnsn_new_progress()
{
  dmnsn_progress_element element = { .progress = 0, .total = 1 };
  dmnsn_progress *progress = malloc(sizeof(dmnsn_progress));
  if (progress) {
    progress->elements = dmnsn_new_array(sizeof(dmnsn_progress_element));
    dmnsn_array_push(progress->elements, &element);

    /* Allocate space for the rwlock, condition variable, and mutex */

    progress->rwlock = malloc(sizeof(pthread_rwlock_t));
    if (!progress->rwlock) {
      dmnsn_delete_array(progress->elements);
      free(progress);
      return NULL;
    }

    progress->cond = malloc(sizeof(pthread_cond_t));
    if (!progress->cond) {
      free(progress->rwlock);
      dmnsn_delete_array(progress->elements);
      free(progress);
      return NULL;
    }

    progress->mutex = malloc(sizeof(pthread_mutex_t));
    if (!progress->mutex) {
      free(progress->rwlock);
      free(progress->cond);
      dmnsn_delete_array(progress->elements);
      free(progress);
      return NULL;
    }

    if (pthread_rwlock_init(progress->rwlock, NULL) != 0) {
      free(progress->rwlock);
      free(progress->mutex);
      free(progress->cond);
      dmnsn_delete_array(progress->elements);
      free(progress);
      return NULL;
    }

    if (pthread_cond_init(progress->cond, NULL) != 0) {
      if (pthread_rwlock_destroy(progress->rwlock) != 0) {
        dmnsn_error(DMNSN_SEVERITY_LOW,
                    "Leaking rwlock in failed allocation.");
      }
      free(progress->rwlock);
      free(progress->mutex);
      free(progress->cond);
      dmnsn_delete_array(progress->elements);
      free(progress);
      return NULL;
    }
    if (pthread_mutex_init(progress->mutex, NULL) != 0) {
      if (pthread_rwlock_destroy(progress->rwlock) != 0) {
        dmnsn_error(DMNSN_SEVERITY_LOW,
                    "Leaking rwlock in failed allocation.");
      }
      if (pthread_cond_destroy(progress->cond) != 0) {
        dmnsn_error(DMNSN_SEVERITY_LOW,
                    "Leaking condition variable in failed allocation.");
      }
      free(progress->rwlock);
      free(progress->mutex);
      free(progress->cond);
      dmnsn_delete_array(progress->elements);
      free(progress);
      return NULL;
    }
  }
  return progress;
}

/* Delete a dmnsn_progress*, which has not yet been associated with a thread;
   mostly for failed returns from *_async() functions. */
void
dmnsn_delete_progress(dmnsn_progress *progress)
{
  if (progress) {
    if (pthread_rwlock_destroy(progress->rwlock) != 0) {
      dmnsn_error(DMNSN_SEVERITY_LOW, "Leaking rwlock.");
    }
    if (pthread_mutex_destroy(progress->mutex) != 0) {
      dmnsn_error(DMNSN_SEVERITY_LOW, "Leaking mutex.");
    }
    if (pthread_cond_destroy(progress->cond) != 0) {
      dmnsn_error(DMNSN_SEVERITY_LOW, "Leaking condition variable.");
    }

    free(progress->mutex);
    free(progress->cond);
    dmnsn_delete_array(progress->elements);
    free(progress);
  }
}

/* Join the worker thread and delete `progress'. */
int dmnsn_finish_progress(dmnsn_progress *progress)
{
  void *ptr;
  int retval = 1;

  if (progress) {
    if (pthread_cond_broadcast(progress->cond) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't signal condition variable.");
    }

    if (pthread_join(progress->thread, &ptr) != 0) {
      /* Medium severity because an unjoined thread likely means that the thread
         is incomplete or invalid */
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Joining worker thread failed.");
    } else if (ptr) {
      retval = *(int *)ptr;
      free(ptr);
    }
    dmnsn_delete_progress(progress);
  }

  return retval;
}

/* Get the current progress of the worker thread, in [0.0, 1.0] */
double
dmnsn_get_progress(const dmnsn_progress *progress)
{
  dmnsn_progress_element *element;
  double prog = 0.0;
  unsigned int i, size;

  dmnsn_progress_rdlock(progress);
    size = dmnsn_array_size(progress->elements);
    for (i = 0; i < size; ++i) {
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
    dmnsn_error(DMNSN_SEVERITY_LOW, "Couldn't lock condition mutex.");
  } else {
    while (dmnsn_get_progress(progress) < prog) {
      if (pthread_cond_wait(progress->cond, progress->mutex) != 0) {
        dmnsn_error(DMNSN_SEVERITY_LOW,
                    "Couldn't wait on condition variable.");
      }
    }

    if (pthread_mutex_unlock(progress->mutex) != 0) {
      dmnsn_error(DMNSN_SEVERITY_LOW, "Couldn't unlock condition mutex.");
    }
  }
}

/* Start a new level of algorithmic nesting */
void
dmnsn_new_progress_element(dmnsn_progress *progress, unsigned int total)
{
  dmnsn_progress_element element = { .progress = 0, .total = total };
  dmnsn_array_push(progress->elements, &element);
}

/* Only the innermost loop needs to call this function - it handles the rest
   upon loop completion (progress == total) */
void
dmnsn_increment_progress(dmnsn_progress *progress)
{
  dmnsn_progress_element *element;
  size_t size;

  dmnsn_progress_wrlock(progress);
    size = dmnsn_array_size(progress->elements);
    element = dmnsn_array_at(progress->elements, size - 1);
    ++element->progress; /* Increment the last element */

    while (element->progress >= element->total && size > 1) {
      /* As long as the last element is complete, pop it */
      --size;
      dmnsn_array_resize(progress->elements, size);
      element = dmnsn_array_at(progress->elements, size - 1);
      ++element->progress; /* Increment the next element */
    }

    if (pthread_cond_broadcast(progress->cond) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't signal condition variable.");
    }
  dmnsn_progress_unlock(progress);
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

    if (pthread_cond_broadcast(progress->cond) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't signal condition variable.");
    }
  dmnsn_progress_unlock(progress);
}

/* Thread synchronization */

static void
dmnsn_progress_rdlock(const dmnsn_progress *progress)
{
  if (pthread_rwlock_rdlock(progress->rwlock) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't acquire read-lock.");
  }
}

static void
dmnsn_progress_wrlock(dmnsn_progress *progress)
{
  if (pthread_rwlock_wrlock(progress->rwlock) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't acquire write-lock.");
  }
}

static void
dmnsn_progress_unlock(const dmnsn_progress *progress)
{
  if (pthread_rwlock_unlock(progress->rwlock) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't unlock read-write lock.");
  }
}
