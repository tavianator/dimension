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

#include "dimension.h"
#include <sys/times.h>
#include <unistd.h>

static long clk_tck = 0;

dmnsn_timer *
dmnsn_new_timer()
{
  /* Figure out the clock ticks per second */
  if (!clk_tck) {
    clk_tck = sysconf(_SC_CLK_TCK);
    if (clk_tck == -1) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "sysconf(_SC_CLK_TCK) failed.");
      clk_tck = 1000000L;
    }
  }

  dmnsn_timer *timer = dmnsn_malloc(sizeof(dmnsn_timer));

  struct tms buf;
  clock_t real = times(&buf);
  timer->real   = (double)real/clk_tck;
  timer->user   = (double)buf.tms_utime/clk_tck;
  timer->system = (double)buf.tms_stime/clk_tck;

  return timer;
}

void
dmnsn_complete_timer(dmnsn_timer *timer)
{
  struct tms buf;
  clock_t real = times(&buf);
  timer->real   = (double)real/clk_tck - timer->real;
  timer->user   = (double)buf.tms_utime/clk_tck - timer->user;
  timer->system = (double)buf.tms_stime/clk_tck - timer->system;
}

void
dmnsn_delete_timer(dmnsn_timer *timer)
{
  dmnsn_free(timer);
}
