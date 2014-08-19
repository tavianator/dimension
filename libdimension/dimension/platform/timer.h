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
 * A platform-agnostic timer abstraction.
 */

#ifndef DMNSN_PLATFORM_H
#error "Please include <dimension/platform.h> instead of this header directly."
#endif

/** A platform-agnotic timer. */
typedef struct dmnsn_timer {
  double real;   /**< Wall-clock time. */
  double user;   /**< Time spent executing. */
  double system; /**< Time spent waiting for the system. */
} dmnsn_timer;

/** A standard format string for timers. */
#define DMNSN_TIMER_FORMAT "%.2fs (user: %.2fs; system: %.2fs)"
/**
 * The appropriate arguments to printf() a timer.  For example:
 * @code
 *   printf(DMNSN_TIMER_FORMAT "\n", DMNSN_TIMER_PRINTF(timer));
 * @endcode
 * will print something like "1.00s (user: 0.99s; system: 0.01s)".
 */
#define DMNSN_TIMER_PRINTF(t) (t).real, (t).user, (t).system

/**
 * Start a timer.  The values of an unfinished timer are undefined.
 * @param[in,out] timer  The timer to start.
 */
void dmnsn_timer_start(dmnsn_timer *timer);

/**
 * Finish timing.  The members of the timer struct will now contain timing data.
 * @param[in,out] timer  The timer to stop.
 */
void dmnsn_timer_stop(dmnsn_timer *timer);
