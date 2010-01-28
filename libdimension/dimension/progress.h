/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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

/*
 * An interface for asynchronous tasks.  *_async() versions of functions
 * return a dmnsn_progress* object which can indicate the progress of the
 * background task, and wait for task completion.  The task's return value
 * is returned as an int from dmnsn_finish_progress().
 */

#ifndef DIMENSION_PROGRESS_H
#define DIMENSION_PROGRESS_H

#include <pthread.h>

/* A single element in an array for dmnsn_progress.  Progress of this item is
   progress/total. */
typedef struct {
  unsigned int progress, total;
} dmnsn_progress_element;

typedef struct {
  /* Array of progress elements.  Progress is given by P(0), where
     P(i) = (elements[i].progress + P(i + 1))/elements[i].total. */
  dmnsn_array *elements;

  /* The worker thread */
  pthread_t thread;

  /* Read-write synchronization */
  pthread_rwlock_t *rwlock;

  /* Condition variable for waiting for a particular amount of progress */
  pthread_cond_t  *cond;
  pthread_mutex_t *mutex;
} dmnsn_progress;

/* Allocate a new progress object */
dmnsn_progress *dmnsn_new_progress();
/* For failed returns from *_async() functions */
void dmnsn_delete_progress(dmnsn_progress *progress);

/* Join the worker thread and returns it's integer return value in addition to
   deleting `progress' */
int dmnsn_finish_progress(dmnsn_progress *progress);

/* Get the progress of the background task, out of 1.0 */
double dmnsn_get_progress(const dmnsn_progress *progress);
/* Wait for the progress to be >= prog, in a better way than spinlocking */
void dmnsn_wait_progress(const dmnsn_progress *progress, double prog);

/* Create a new level of loop nesting */
void dmnsn_new_progress_element(dmnsn_progress *progress, unsigned int total);
/* Increment the progress counter; should only be called from innermost loop */
void dmnsn_increment_progress(dmnsn_progress *progress);
/* Instantly complete the progress */
void dmnsn_done_progress(dmnsn_progress *progress);

#endif /* DIMENSION_PROGRESS_H */
