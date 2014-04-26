/*************************************************************************
 * Copyright (C) 2013-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

static void
dmnsn_bench_future_increment(void)
{
  dmnsn_future *future = dmnsn_new_future();
  dmnsn_future_set_total(future, ITERATIONS);

  sandglass_t sandglass;
  if (sandglass_init_monotonic(&sandglass, SANDGLASS_CPUTIME) != 0) {
    perror("sandglass_create()");
    exit(EXIT_FAILURE);
  }

  /* Benchmark the increment operation. */
  sandglass_bench_fine(&sandglass, dmnsn_future_increment(future));
  printf("dmnsn_future_increment(): %ld\n", sandglass.grains);

  dmnsn_delete_future(future);
}

static int
dmnsn_bench_future_ccthread(void *ptr, unsigned int thread,
                            unsigned int nthreads)
{
  dmnsn_future *future = (dmnsn_future *)ptr;
  for (int i = 0; i < ITERATIONS; ++i) {
    dmnsn_future_increment(future);
  }
  return 0;
}

static int
dmnsn_bench_future_thread(void *ptr)
{
  dmnsn_future *future = (dmnsn_future *)ptr;
  size_t nthreads = 2*dmnsn_ncpus();
  dmnsn_future_set_total(future, nthreads*ITERATIONS);

  /* Now run a bunch of increments concurrently. */
  return dmnsn_execute_concurrently(future, dmnsn_bench_future_ccthread,
                                    future, nthreads);
}

static void
dmnsn_bench_future_concurrent(void)
{
  printf("\nNo pausing:\n");

  dmnsn_future *future = dmnsn_new_future();

  dmnsn_timer timer1, timer2;
  dmnsn_timer_start(&timer1);
  timer2 = timer1;

  dmnsn_new_thread(future, dmnsn_bench_future_thread, future);

  dmnsn_future_wait(future, 0.5);
  dmnsn_timer_stop(&timer1);
  printf(" 50%%: " DMNSN_TIMER_FORMAT "\n", DMNSN_TIMER_PRINTF(timer1));

  dmnsn_future_join(future);
  dmnsn_timer_stop(&timer2);
  printf("100%%: " DMNSN_TIMER_FORMAT "\n", DMNSN_TIMER_PRINTF(timer2));
}

static void
dmnsn_bench_future_pausing(void)
{
  printf("\nWith pausing:\n");

  dmnsn_future *future = dmnsn_new_future();
  bool hit_fifty = false;

  dmnsn_timer timer1, timer2;
  dmnsn_timer_start(&timer1);
  timer2 = timer1;

  dmnsn_new_thread(future, dmnsn_bench_future_thread, future);

  while (!dmnsn_future_is_done(future)) {
    dmnsn_future_pause(future);
      if (dmnsn_future_progress(future) >= 0.5 && !hit_fifty) {
        dmnsn_timer_stop(&timer1);
        printf(" 50%%: " DMNSN_TIMER_FORMAT "\n", DMNSN_TIMER_PRINTF(timer1));
        hit_fifty = true;
      }
    dmnsn_future_resume(future);
  }

  dmnsn_future_join(future);
  dmnsn_timer_stop(&timer2);
  printf("100%%: " DMNSN_TIMER_FORMAT "\n", DMNSN_TIMER_PRINTF(timer2));
}

int
main(void)
{
  dmnsn_bench_future_increment();
  dmnsn_bench_future_concurrent();
  dmnsn_bench_future_pausing();
  return EXIT_SUCCESS;
}
