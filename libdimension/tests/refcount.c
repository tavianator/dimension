/*************************************************************************
 * Copyright (C) 2012 Tavian Barnes <tavianator@tavianator.com>          *
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
 * Tests for the reference counting implementation.
 */

#include "dimension-internal.h"
#include "tests.h"

typedef struct dmnsn_foo {
  DMNSN_REFCOUNT;
} dmnsn_foo;

static dmnsn_foo *
dmnsn_new_foo(void)
{
  dmnsn_foo *foo = dmnsn_malloc(sizeof(dmnsn_foo));
  DMNSN_REFCOUNT_INIT(foo);
  return foo;
}

static void
dmnsn_delete_foo(dmnsn_foo *foo)
{
  if (DMNSN_DECREF(foo)) {
    dmnsn_free(foo);
  }
}

static dmnsn_foo dmnsn_global_foo = {
  DMNSN_REFCOUNT_INITIALIZER,
};

DMNSN_TEST(refcount, inc_dec)
{
  dmnsn_foo *foo = dmnsn_new_foo();
  DMNSN_INCREF(foo);
  dmnsn_delete_foo(foo);
  dmnsn_delete_foo(foo);
}

DMNSN_TEST(refcount, decref_null)
{
  dmnsn_foo *foo = NULL;
  ck_assert(!DMNSN_DECREF(foo));
}

DMNSN_TEST(refcount, global)
{
  /* Suppress "address will always evaluate to true" warning */
  dmnsn_foo *global = &dmnsn_global_foo;
  DMNSN_INCREF(global);
  ck_assert(!DMNSN_DECREF(global));
}
