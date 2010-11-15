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

/**
 * @file
 * An interface for asynchronous tasks.  *_async() versions of functions
 * return a dmnsn_progress* object which can indicate the progress of the
 * background task, and wait for task completion.  The task's return value
 * is returned as an int from dmnsn_finish_progress().
 */

#ifndef DIMENSION_PROGRESS_H
#define DIMENSION_PROGRESS_H

/** A progress object. */
typedef struct dmnsn_progress dmnsn_progress;

/**
 * Join the worker thread and return it's integer return value in addition to
 * deleting \p progress.
 * @param[in,out] progress  The background task to finish.
 * @return The return value of the background task.
 */
int dmnsn_finish_progress(dmnsn_progress *progress);

/**
 * Get the progress of the background task.
 * @param[in] progress  The background task to examine.
 * @return The progress of the background task, out of 1.0.
 */
double dmnsn_get_progress(const dmnsn_progress *progress);

/**
 * Wait for a certain amount of progress.  Always use this rather than
 * spinlocking.
 * @param[in] progress  The background task to monitor.
 * @param[in] prog      The progress value to wait for.
 */
void dmnsn_wait_progress(const dmnsn_progress *progress, double prog);

#endif /* DIMENSION_PROGRESS_H */
