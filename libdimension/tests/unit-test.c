/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Shared entry point for tests based on the Check unit-testing framework.
 */

#include "tests.h"

dmnsn_dictionary *dmnsn_test_cases = NULL;

TCase *
dmnsn_get_test_case(const char* name)
{
  if (!dmnsn_test_cases) {
    dmnsn_test_cases = dmnsn_new_dictionary(sizeof(TCase *));
  }

  TCase **tcp = dmnsn_dictionary_at(dmnsn_test_cases, name);
  if (tcp) {
    return *tcp;
  }

  TCase *tc = tcase_create(name);
  tcase_add_checked_fixture(tc, dmnsn_test_setup, dmnsn_test_teardown);
  dmnsn_dictionary_insert(dmnsn_test_cases, name, &tc);
  return tc;
}

void
dmnsn_test_setup(void)
{
  // Treat warnings as errors for tests
  dmnsn_die_on_warnings(true);
}

void
dmnsn_test_teardown(void)
{
}

__attribute__((destructor))
static void
dmnsn_test_cleanup(void)
{
  /* Can't go in dmnsn_test_teardown(), because it should run even if the test
     fails. */
  dmnsn_delete_dictionary(dmnsn_test_cases);
}

static Suite *dmnsn_suite;

void
dmnsn_add_test_cases(void *ptr)
{
  TCase **tcp = ptr;
  suite_add_tcase(dmnsn_suite, *tcp);
}

Suite *
dmnsn_test_suite(void)
{
  dmnsn_suite = suite_create("Dimension");

  if (dmnsn_test_cases) {
    dmnsn_dictionary_apply(dmnsn_test_cases, dmnsn_add_test_cases);
  }

  return dmnsn_suite;
}

int
main(void)
{
  // Treat warnings as errors for tests
  dmnsn_die_on_warnings(true);

  // Create the test suite
  Suite *suite = dmnsn_test_suite();
  SRunner *sr = srunner_create(suite);

  // Run the tests
  srunner_run_all(sr, CK_VERBOSE);
  int nfailed = srunner_ntests_failed(sr);

  // Clean up
  srunner_free(sr);

  // Return the right result code
  if (nfailed == 0) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
