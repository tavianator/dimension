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

  /* Condition variable for waiting for a particular amount of progress */
  pthread_cond_t  *cond;
  pthread_mutex_t *mutex;
} dmnsn_progress;

dmnsn_progress *dmnsn_new_progress();
void dmnsn_delete_progress(dmnsn_progress *progress);

/* This joins the worker thread and returns it's integer return value in
   addition to deleting `progress' */
int dmnsn_finish_progress(dmnsn_progress *progress);

double dmnsn_get_progress(const dmnsn_progress *progress);
void dmnsn_wait_progress(const dmnsn_progress *progress, double prog);

void dmnsn_new_progress_element(dmnsn_progress *progress, unsigned int total);
void dmnsn_increment_progress(dmnsn_progress *progress);
void dmnsn_progress_done(dmnsn_progress *progress);

#endif /* DIMENSION_PROGRESS_H */
