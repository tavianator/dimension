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
 * Tests for memory pools.
 */

#include "../dimension-internal.h"
#include "tests.h"
#include "../future.c"
#include "../threads.c"

static dmnsn_pool *pool;

DMNSN_TEST_SETUP(pool)
{
  pool = dmnsn_new_pool();
}

DMNSN_TEST_TEARDOWN(pool)
{
  dmnsn_delete_pool(pool);
}

DMNSN_TEST(pool, simple)
{
  for (int i = 0; i < 10000; ++i) {
    int *p = DMNSN_PALLOC(pool, int);
    *p = i;
  }

  // Leak checking will tell us if something bad happened
}

static int counter = 0;

static void
callback(void *ptr)
{
  ++counter;
}

DMNSN_TEST(pool, callback)
{
  DMNSN_PALLOC_TIDY(pool, int, NULL);
  DMNSN_PALLOC_TIDY(pool, int, callback);
  DMNSN_PALLOC_TIDY(pool, int, callback);
  DMNSN_PALLOC_TIDY(pool, int, NULL);

  dmnsn_delete_pool(pool);
  pool = NULL;

  ck_assert_int_eq(counter, 2);
}

static int
alloc_thread(void *ptr, unsigned int thread, unsigned int nthreads)
{
  for (unsigned int i = thread; i < 10000; i += nthreads) {
    int *p = DMNSN_PALLOC(pool, int);
    *p = i;
  }
  return 0;
}

DMNSN_TEST(pool, threaded)
{
  int ret = dmnsn_execute_concurrently(NULL, alloc_thread, NULL, 64);
  ck_assert_int_eq(ret, 0);
}
