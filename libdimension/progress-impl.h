/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Progress object implementation.
 */

#ifndef DIMENSION_IMPL_PROGRESS_H
#define DIMENSION_IMPL_PROGRESS_H

#include <pthread.h>

/** Allocate a new progress object. */
DMNSN_INTERNAL dmnsn_progress *dmnsn_new_progress(void);

/** Set the total number of loop iterations. */
DMNSN_INTERNAL void dmnsn_set_progress_total(dmnsn_progress *progress,
                                             size_t total);
/** Increment the progress counter. */
DMNSN_INTERNAL void dmnsn_increment_progress(dmnsn_progress *progress);
/** Instantly complete the progress. */
DMNSN_INTERNAL void dmnsn_done_progress(dmnsn_progress *progress);

struct dmnsn_progress {
  size_t progress; /**< Completed loop iterations. */
  size_t total;    /**< Total expected loop iterations. */

  /* The worker thread */
  pthread_t thread;

  /* Read-write synchronization */
  pthread_rwlock_t *rwlock;

  /* Condition variable for waiting for a particular amount of progress */
  pthread_cond_t  *cond;
  pthread_mutex_t *mutex;

  /* Minimum waited-on value */
  volatile double min_wait;
  volatile double *min_waitp; /* Hack for const values */
};

#endif /* DIMENSION_IMPL_PROGRESS_H */
