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
 * Tests for dmnsn_dictionary, a string->object map.
 */

#include "tests.h"

static dmnsn_dictionary *dict;

/*
 * Dictionary creation/deletion.
 */

DMNSN_TEST(new_delete, delete_null)
{
  dmnsn_delete_dictionary(NULL);
}

/*
 * Dictionary operations.
 */

DMNSN_TEST_SETUP(dictionary)
{
  dict = dmnsn_new_dictionary(sizeof(int));
}

DMNSN_TEST_TEARDOWN(dictionary)
{
  dmnsn_delete_dictionary(dict);
}

DMNSN_TEST(dictionary, insert_and_get)
{
  const int value_in = 123;
  dmnsn_dictionary_insert(dict, "key", &value_in);

  int value_out;
  ck_assert(dmnsn_dictionary_get(dict, "key", &value_out));
  ck_assert_int_eq(value_out, value_in);

  int *value_at = dmnsn_dictionary_at(dict, "key");
  ck_assert_int_eq(*value_at, value_in);
}

DMNSN_TEST(dictionary, insert_multiple)
{
  const int value1 = 123, value2 = 456, value3 = 789;
  int value_out;

  dmnsn_dictionary_insert(dict, "key1", &value1);
  dmnsn_dictionary_insert(dict, "key2", &value2);
  dmnsn_dictionary_insert(dict, "asdf", &value3);

  ck_assert(dmnsn_dictionary_get(dict, "key1", &value_out));
  ck_assert_int_eq(value_out, value1);

  ck_assert(dmnsn_dictionary_get(dict, "key2", &value_out));
  ck_assert_int_eq(value_out, value2);

  ck_assert(dmnsn_dictionary_get(dict, "asdf", &value_out));
  ck_assert_int_eq(value_out, value3);
}

DMNSN_TEST(dictionary, insert_overwrites)
{
  const int value1 = 123, value2 = 456;
  int value_out;

  /* Insert and read back value1 */
  dmnsn_dictionary_insert(dict, "key", &value1);
  ck_assert(dmnsn_dictionary_get(dict, "key", &value_out));
  ck_assert_int_eq(value_out, value1);

  /* Insert and read back value2 */
  dmnsn_dictionary_insert(dict, "key", &value2);
  ck_assert(dmnsn_dictionary_get(dict, "key", &value_out));
  ck_assert_int_eq(value_out, value2);
}

DMNSN_TEST(dictionary, remove)
{
  const int value_in = 123;
  dmnsn_dictionary_insert(dict, "key", &value_in);
  ck_assert(dmnsn_dictionary_remove(dict, "key"));
  ck_assert(!dmnsn_dictionary_remove(dict, "key"));

  int value_out;
  ck_assert(!dmnsn_dictionary_get(dict, "key", &value_out));

  int *value_at = dmnsn_dictionary_at(dict, "key");
  ck_assert(value_at == NULL);
}

static int sum = 0;

static void
dmnsn_dictionary_test_apply_callback(void *ptr)
{
  sum += *(int *)ptr;
}

DMNSN_TEST(dictionary, apply)
{
  const int value1 = 123, value2 = 456, value3 = 789;

  dmnsn_dictionary_insert(dict, "key1", &value1);
  dmnsn_dictionary_insert(dict, "key2", &value2);
  dmnsn_dictionary_insert(dict, "asdf", &value3);

  dmnsn_dictionary_apply(dict, dmnsn_dictionary_test_apply_callback);
  ck_assert_int_eq(sum, value1 + value2 + value3);
}
