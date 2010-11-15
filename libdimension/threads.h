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

/**
 * @file
 * Background threading interface.
 */

#ifndef DIMENSION_IMPL_THREADS_H
#define DIMENSION_IMPL_THREADS_H

/**
 * Thread callback type.
 * @param[in,out] ptr  An arbitrary pointer.
 * @return 0 on success, non-zero on failure.
 */
typedef int dmnsn_thread_fn(void *ptr);

/**
 * Create a thread that cleans up after itself on errors.
 * @param[in,out] progress   The progress object to associate with the thread.
 * @param[in]     thread_fn  The thread callback.
 * @param[in,out] arg        The pointer to pass to the thread callback.
 */
void dmnsn_new_thread(dmnsn_progress *progress, dmnsn_thread_fn *thread_fn,
                      void *arg);

#endif /* DIMENSION_IMPL_THREADS_H */
