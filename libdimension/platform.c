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

#include "dimension_impl.h"
#include <unistd.h> /* For sysconf() */
#include <sched.h>  /* For sched_getaffinity() */

size_t
dmnsn_ncpus()
{
  cpu_set_t cpuset;
  if (sched_getaffinity(0, sizeof(cpuset), &cpuset) == 0) {
    return CPU_COUNT(&cpuset);
  } else {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "sched_getaffinity() failed.");
    return 1;
  }
}
