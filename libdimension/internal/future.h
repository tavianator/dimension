/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

#ifndef DMNSN_INTERNAL_FUTURE_H
#define DMNSN_INTERNAL_FUTURE_H

#include <pthread.h>

struct dmnsn_future {
  size_t progress; ///< Completed loop iterations.
  size_t total;    ///< Total expected loop iterations.

  /// The worker thread.
  pthread_t thread;

  /// Mutex to guard progress and total.
  pthread_mutex_t mutex;

  /// Condition variable for waiting for a particular amount of progress.
  pthread_cond_t cond;

  /// Minimum waited-on value.
  double min_wait;

  /// Number of threads working on the future's background task.
  unsigned int nthreads;
  /// Number of threads not yet paused.
  unsigned int nrunning;
  /// Count of threads holding the future paused.
  unsigned int npaused;
  /// Condition variable for waiting for nrunning == 0.
  pthread_cond_t none_running_cond;
  /// Condition variable for waiting for nrunning == nthreads.
  pthread_cond_t all_running_cond;
  /// Condition variable for waiting for npaused == 0.
  pthread_cond_t resume_cond;
};

#endif // DMNSN_INTERNAL_FUTURE_H
