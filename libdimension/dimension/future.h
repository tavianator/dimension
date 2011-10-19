/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * return a dmnsn_future* object which can indicate the progress of the
 * background task, and wait for task completion.  The task's return value
 * is returned as an int from dmnsn_finish_progress().
 */

/** A future object. */
typedef struct dmnsn_future dmnsn_future;

/**
 * Join the worker thread and return its integer return value in addition to
 * deleting \p future.
 * @param[in,out] future  The background task to join.
 * @return The return value of the background task.
 */
int dmnsn_future_join(dmnsn_future *future);

/**
 * Interrupt the execution of a background thread.
 * @param[in,out] future  The background task to cancel.
 */
void dmnsn_future_cancel(dmnsn_future *future);

/**
 * Get the progress of the background task.
 * @param[in] future  The background task to examine.
 * @return The progress of the background task, in [0.0, 1.0].
 */
double dmnsn_future_progress(const dmnsn_future *future);

/**
 * Wait for a certain amount of progress.  Always use this rather than
 * spinlocking.
 * @param[in] future    The background task to monitor.
 * @param[in] progress  The progress value to wait for.
 */
void dmnsn_future_wait(const dmnsn_future *future, double progress);
