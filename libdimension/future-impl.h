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
 * Future object implementation.
 */

#include <pthread.h>

/** Allocate a new future object. */
DMNSN_INTERNAL dmnsn_future *dmnsn_new_future(void);

/** Set the total number of loop iterations. */
DMNSN_INTERNAL void dmnsn_future_set_total(dmnsn_future *future, size_t total);
/** Increment the progress of a background task. */
DMNSN_INTERNAL void dmnsn_future_increment(dmnsn_future *future);
/** Instantly complete the background teask. */
DMNSN_INTERNAL void dmnsn_future_done(dmnsn_future *future);

struct dmnsn_future {
  size_t progress; /**< Completed loop iterations. */
  size_t total;    /**< Total expected loop iterations. */

  /** The worker thread. */
  pthread_t thread;

  /** Read-write synchronization. */
  pthread_rwlock_t *rwlock;

  /** Condition variable for waiting for a particular amount of progress. */
  pthread_cond_t  *cond;
  pthread_mutex_t *mutex;

  /** Minimum waited-on value. */
  double *min_wait;
};
