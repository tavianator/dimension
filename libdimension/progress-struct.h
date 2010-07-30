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

#ifndef DIMENSION_IMPL_PROGRESS_STRUCT_H
#define DIMENSION_IMPL_PROGRESS_STRUCT_H

struct dmnsn_progress {
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
};

#endif /* DIMENSION_IMPL_PROGRESS_STRUCT_H */