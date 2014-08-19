/*************************************************************************
 * Copyright (C) 2014 Tavian Barnes <tavianator@tavianator.com>          *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * The Dimension Test Suite is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License as *
 * published by the Free Software Foundation; either version 3 of the    *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Test Suite is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

/**
 * @file
 * Tests for dmnsn_future.
 */

#include "../platform/platform.c"
#include "../concurrency/future.c"
#include "../concurrency/threads.c"
#include "tests.h"
#include <stdatomic.h>

static dmnsn_future *future;
static atomic_int counter = ATOMIC_VAR_INIT(0);
static const int NTHREADS = 8;
static const int CHUNK = 100;

static int
dmnsn_future_test_ccthread(void *ptr, unsigned int thread, unsigned int nthreads)
{
  for (int i = 0; i < CHUNK; ++i) {
    for (int j = 0; j < CHUNK; ++j) {
      atomic_fetch_add(&counter, 1);
    }
    dmnsn_future_increment(future);
  }
  return 0;
}

static int
dmnsn_future_test_thread(void *ptr)
{
  dmnsn_future_set_total(future, NTHREADS*CHUNK);
  return dmnsn_execute_concurrently(future, dmnsn_future_test_ccthread, ptr, NTHREADS);
}

DMNSN_TEST_SETUP(future)
{
  future = dmnsn_new_future();
  dmnsn_new_thread(future, dmnsn_future_test_thread, NULL);
}

DMNSN_TEST_TEARDOWN(future)
{
  int res = dmnsn_future_join(future);
  ck_assert_int_eq(res, 0);
}

void *
dmnsn_future_pause_and_check(void *ptr)
{
  for (int i = 0; i < CHUNK; ++i) {
    dmnsn_future_pause(future);

      int value = atomic_load(&counter);
      ck_assert_int_eq(value%CHUNK, 0);

      for (int j = 0; j < CHUNK; ++j) {
        int new_value = atomic_load(&counter);
        ck_assert_int_eq(new_value, value);
      }

    dmnsn_future_resume(future);
  }

  return NULL;
}

DMNSN_TEST(future, pause_safely)
{
  // Check dmnsn_future_pause() behaviour from two concurrent threads
  pthread_t thread;
  if (pthread_create(&thread, NULL, dmnsn_future_pause_and_check, NULL) != 0) {
    dmnsn_error("Couldn't start thread.");
  }

  dmnsn_future_pause_and_check(NULL);

  dmnsn_join_thread(thread, NULL);
}
