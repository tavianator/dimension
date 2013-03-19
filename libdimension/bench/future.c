/*************************************************************************
 * Copyright (C) 2009-2013 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Benchmark Suite.                   *
 *                                                                       *
 * The Dimension Benchmark Suite is free software; you can redistribute  *
 * it and/or modify it under the terms of the GNU General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Benchmark Suite is distributed in the hope that it will *
 * be useful, but WITHOUT ANY WARRANTY; without even the implied         *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See *
 * the GNU General Public License for more details.                      *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "../platform.c"
#include "../future.c"
#include "../threads.c"
#include <sandglass.h>
#include <stdlib.h>

#define ITERATIONS 100000

static int
dmnsn_bench_future(void *ptr, unsigned int thread, unsigned int nthreads)
{
  dmnsn_future *future = (dmnsn_future *)ptr;
  for (int i = 0; i < ITERATIONS; ++i) {
    dmnsn_future_increment(future);
  }
  return 0;
}

static int
dmnsn_bench_thread(void *ptr)
{
  size_t nthreads = 2*dmnsn_ncpus();

  dmnsn_future *future = (dmnsn_future *)ptr;
  dmnsn_future_set_total(future, nthreads*ITERATIONS);

  return dmnsn_execute_concurrently(&dmnsn_bench_future, future, nthreads);
}

int
main(void)
{
  dmnsn_timer timer1, timer2;
  dmnsn_future *future = dmnsn_new_future();

  dmnsn_timer_start(&timer1);
  dmnsn_timer_start(&timer2);

  dmnsn_new_thread(future, &dmnsn_bench_thread, future);

  dmnsn_future_wait(future, 0.5);
  dmnsn_timer_stop(&timer1);
  printf(" 50%%: " DMNSN_TIMER_FORMAT "\n", DMNSN_TIMER_PRINTF(timer1));

  dmnsn_future_join(future);
  dmnsn_timer_stop(&timer2);
  printf("100%%: " DMNSN_TIMER_FORMAT "\n", DMNSN_TIMER_PRINTF(timer2));
}
