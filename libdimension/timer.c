/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Performance counter.
 */

#include "dimension-impl.h"

void
dmnsn_start_timer(dmnsn_timer *timer)
{
  dmnsn_get_times(timer);
}

void
dmnsn_stop_timer(dmnsn_timer *timer)
{
  dmnsn_timer now;
  dmnsn_get_times(&now);
  timer->real   = now.real   - timer->real;
  timer->user   = now.user   - timer->user;
  timer->system = now.system - timer->system;
}
