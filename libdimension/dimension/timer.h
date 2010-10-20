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
 * A platform-agnostic timer abstraction
 */

typedef struct dmnsn_timer {
  double real, user, system;
} dmnsn_timer;

#define DMNSN_TIMER_FORMAT "%.2fs (user: %.2fs; system: %.2fs)"
#define DMNSN_TIMER_PRINTF(t) (t)->real, (t)->user, (t)->system

dmnsn_timer *dmnsn_new_timer(void);
void dmnsn_complete_timer(dmnsn_timer *timer);
void dmnsn_delete_timer(dmnsn_timer *timer);
