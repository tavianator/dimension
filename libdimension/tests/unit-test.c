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

void
dmnsn_test_setup(void)
{
  /* Treat warnings as errors for tests */
  dmnsn_die_on_warnings(true);
}

void
dmnsn_test_teardown(void)
{
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
dmnsn_test_suite()
{
  dmnsn_suite = suite_create("Dimension");

  if (dmnsn_test_cases != NULL) {
    dmnsn_dictionary_apply(dmnsn_test_cases, dmnsn_add_test_cases);
  }

  return dmnsn_suite;
}

int
main()
{
  /* Treat warnings as errors for tests */
  dmnsn_die_on_warnings(true);

  /* Create the test suite */
  Suite *suite = dmnsn_test_suite();
  SRunner *sr = srunner_create(suite);

  /* Run the tests */
  srunner_run_all(sr, CK_VERBOSE);
  int nfailed = srunner_ntests_failed(sr);

  /* Clean up */
  srunner_free(sr);
  dmnsn_delete_dictionary(dmnsn_test_cases);

  /* Return the right result code */
  if (nfailed == 0) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
