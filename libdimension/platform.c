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
 * Platform abstractions.
 */

#define _GNU_SOURCE
#include "dimension-internal.h"

#if HAVE_UNISTD_H
  #include <unistd.h>
#endif
#ifdef _WIN32
  #include <windows.h>
#endif
#if DMNSN_BACKTRACE
  #include <execinfo.h>    /* For backtrace() etc. */
#endif
#if DMNSN_GETTID
  #include <sys/syscall.h> /* For gettid() where supported */
#endif
#if DMNSN_SCHED_GETAFFINITY
  #include <sched.h>       /* For sched_getaffinity() */
#endif
#if DMNSN_TIMES
  #include <sys/times.h>
#endif
#if DMNSN_GETRUSAGE
  #include <sys/time.h>
  #include <sys/resource.h>
#endif

void
dmnsn_backtrace(FILE *file)
{
#if DMNSN_BACKTRACE
  const size_t size = 128;
  void *buffer[size];

  int nptrs = backtrace(buffer, size);
  int fd = fileno(file);
  if (fd != -1) {
    backtrace_symbols_fd(buffer, nptrs, fd);
  }
#endif
}

bool
dmnsn_is_main_thread(void)
{
#if DMNSN_GETTID
  return getpid() == syscall(SYS_gettid);
#else
  return true;
#endif
}

bool
dmnsn_is_little_endian(void)
{
  /* Undefined behaviour, but quite portable */
  union {
    unsigned int i;
    unsigned char c;
  } u = { .i = 1 };
  return u.c == 1;
}

size_t
dmnsn_ncpus(void)
{
#if DMNSN_SCHED_GETAFFINITY
  cpu_set_t cpuset;
  if (sched_getaffinity(0, sizeof(cpuset), &cpuset) == 0) {
    return CPU_COUNT(&cpuset);
  } else {
    dmnsn_warning("sched_getaffinity() failed.");
    return 1;
  }
#elif DMNSN_SC_NPROCESSORS_ONLN
  long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
  if (nprocs > 0) {
    return nprocs;
  } else {
    dmnsn_warning("sysconf(_SC_NPROCESSORS_ONLN) failed.");
    return 1;
  }
#elif defined(_WIN32)
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;
#else
  return 1;
#endif
}

#if DMNSN_GETRUSAGE
/** Convert a struct timeval to a double. */
static inline double
dmnsn_timeval2double(struct timeval tv)
{
  return tv.tv_sec + tv.tv_usec/1.0e6;
}
#endif

void
dmnsn_get_times(dmnsn_timer *timer)
{
#if DMNSN_GETRUSAGE
  struct timeval real;
  gettimeofday(&real, NULL);

  struct rusage usage;
  if (getrusage(RUSAGE_SELF, &usage) == 0) {
    timer->real   = dmnsn_timeval2double(real);
    timer->user   = dmnsn_timeval2double(usage.ru_utime);
    timer->system = dmnsn_timeval2double(usage.ru_stime);
  } else {
    dmnsn_warning("getrusage() failed.");
    timer->real = timer->user = timer->system = 0.0;
  }
#elif DMNSN_TIMES
  static long clk_tck = 0;

  /* Figure out the clock ticks per second */
  if (!clk_tck) {
    clk_tck = sysconf(_SC_CLK_TCK);
    if (clk_tck == -1) {
      dmnsn_warning("sysconf(_SC_CLK_TCK) failed.");
      clk_tck = 1000000L;
    }
  }

  struct tms buf;
  clock_t real = times(&buf);
  if (real == (clock_t)-1) {
    dmnsn_warning("times() failed.");
    timer->real = timer->user = timer->system = 0.0;
  } else {
    timer->real   = (double)real/clk_tck;
    timer->user   = (double)buf.tms_utime/clk_tck;
    timer->system = (double)buf.tms_stime/clk_tck;
  }
#elif defined(_WIN32)
  FILETIME real;
  GetSystemTimeAsFileTime(&real);

  FILETIME user, system, creation, exit;
  HANDLE current_process = GetCurrentProcess();
  if (GetProcessTimes(current_process,
                      &creation, &exit, &system, &user) != 0)
  {
    timer->real
      = (((uint64_t)real.dwHighDateTime << 32) + real.dwLowDateTime)/1.0e7;
    timer->user
      = (((uint64_t)user.dwHighDateTime << 32) + user.dwLowDateTime)/1.0e7;
    timer->system
      = (((uint64_t)system.dwHighDateTime << 32) + system.dwLowDateTime)/1.0e7;
  } else {
    dmnsn_warning("GetProcessTimes() failed.");
    timer->real = timer->user = timer->system = 0.0;
  }
#else
  timer->real = timer->user = timer->system = 0.0;
#endif
}
