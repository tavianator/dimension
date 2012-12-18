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

/** @internal Get the test case with the given name, possibly creating it. */
TCase *dmnsn_get_test_case(const char* name);

/** @internal Default test fixture. */
void dmnsn_test_setup(void);
/** @internal Default test fixture. */
void dmnsn_test_teardown(void);

/**
 * Defines a test.
 * @param[in] tcase  The name of the test case for this test.
 * @param[in] test   The name of the test itself.
 */
#define DMNSN_TEST(tcase, test)                 \
  DMNSN_TEST_IMPL(#tcase, tcase##_test_##test)

#define DMNSN_TEST_IMPL(tcase, test)            \
  DMNSN_TEST_IMPL2(tcase,                       \
                   dmnsn_##test,                \
                   dmnsn_##test##_impl,         \
                   dmnsn_add_##test)

#define DMNSN_TEST_IMPL2(tcase, test, test_impl, add_test)      \
  static void test(int _i);                                     \
  static void test_impl(int _i);                                \
                                                                \
  __attribute__((constructor))                                  \
  static void                                                   \
  add_test(void)                                                \
  {                                                             \
    TCase *tc = dmnsn_get_test_case(tcase);                     \
    tcase_add_test(tc, test);                                   \
  }                                                             \
                                                                \
  START_TEST(test)                                              \
  {                                                             \
    test_impl(_i);                                              \
  }                                                             \
  END_TEST                                                      \
                                                                \
  static void                                                   \
  test_impl(int _i)

/**
 * Defines the setup method for a test case.
 * @param[in] tcase  The name of the test case.
 */
#define DMNSN_TEST_SETUP(tcase)                                 \
  DMNSN_TEST_FIXTURE(#tcase, tcase##_test_fixture_setup, true)

/**
 * Defines the teardown method for a test case.
 * @param[in] tcase  The name of the test case.
 */
#define DMNSN_TEST_TEARDOWN(tcase)                                      \
  DMNSN_TEST_FIXTURE(#tcase, tcase##_test_fixture_teardown, false)

#define DMNSN_TEST_FIXTURE(tcase, fixture, is_setup)                     \
  DMNSN_TEST_FIXTURE2(tcase, dmnsn_##fixture, dmnsn_add_##fixture, is_setup)

#define DMNSN_TEST_FIXTURE2(tcase, fixture, add_fixture, is_setup)      \
  static void fixture(void);                                            \
                                                                        \
  __attribute__((constructor))                                          \
  static void                                                           \
  add_fixture(void)                                                     \
  {                                                                     \
    TCase *tc = dmnsn_get_test_case(tcase);                             \
    tcase_add_checked_fixture(tc,                                       \
                              is_setup ? fixture : NULL,                \
                              !is_setup ? fixture : NULL);              \
  }                                                                     \
                                                                        \
  static void                                                           \
  fixture(void)

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
