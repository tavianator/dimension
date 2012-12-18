/*************************************************************************
 * Copyright (C) 2009-2012 Tavian Barnes <tavianator@tavianator.com>     *
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

#ifndef TESTS_H
#define TESTS_H

#include "dimension.h"
#include <check.h>

#ifdef __cplusplus
/* We've been included from a C++ file; mark everything here as extern "C" */
extern "C" {
#endif

/** @internal Map to known test cases from their names. */
extern dmnsn_dictionary* dmnsn_test_cases;

/** @internal Default test fixture. */
void dmnsn_test_setup(void);
/** @internal Default test fixture. */
void dmnsn_test_teardown(void);

/**
 * Mark the beginning of a test.
 * @param[in] tcase  The name of the test case for this test.
 * @param[in] test   The name of the test itself.
 */
#define DMNSN_TEST(tcase, test)                                         \
  static void dmnsn_test_##test(int _i);                                \
                                                                        \
  __attribute__((constructor))                                          \
  static void dmnsn_add_test_##test(void)                               \
  {                                                                     \
    if (dmnsn_test_cases == NULL) {                                     \
      dmnsn_test_cases = dmnsn_new_dictionary(sizeof(TCase *));         \
    }                                                                   \
                                                                        \
    TCase *tc;                                                          \
    TCase **tcp = dmnsn_dictionary_at(dmnsn_test_cases, tcase);         \
    if (tcp == NULL) {                                                  \
      tc = tcase_create(tcase);                                         \
      tcase_add_checked_fixture(tc, dmnsn_test_setup, dmnsn_test_teardown); \
      dmnsn_dictionary_insert(dmnsn_test_cases, tcase, &tc);            \
    } else {                                                            \
      tc = *tcp;                                                        \
    }                                                                   \
                                                                        \
    tcase_add_test(tc, dmnsn_test_##test);                              \
  }                                                                     \
                                                                        \
  START_TEST(dmnsn_test_##test)

/** Mark the end of a test. */
#define DMNSN_END_TEST END_TEST

/*
 * Test canvas
 */
void dmnsn_paint_test_canvas(dmnsn_canvas *canvas);

/*
 * Windowing
 */

typedef struct dmnsn_display dmnsn_display;

dmnsn_display *dmnsn_new_display(const dmnsn_canvas *canvas);
void dmnsn_delete_display(dmnsn_display *display);

/* Flush the GL buffers */
void dmnsn_display_flush(dmnsn_display *display);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_H */
