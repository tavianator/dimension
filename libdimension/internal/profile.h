/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Library.                           *
 *                                                                       *
 * The Dimension Library is free software; you can redistribute it and/  *
 * or modify it under the terms of the GNU Lesser General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Library is distributed in the hope that it will be      *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

/**
 * @file
 * Built-in branch profiler.
 */

#ifndef DMNSN_INTERNAL_H
#error "Please include \"internal.h\" instead of this header directly."
#endif

#include <stdbool.h>

/**
 * @def dmnsn_likely
 * Indicate that a test is likely to succeed.
 * @param test  The test to perform.
 * @return The truth value of \p test.
 */
/**
 * @def dmnsn_unlikely
 * Indicate that a test is unlikely to succeed.
 * @param test  The test to perform.
 * @return The truth value of \p test.
 */
#ifdef DMNSN_PROFILE
  #define dmnsn_likely(test)                                      \
    dmnsn_expect(!!(test), true, DMNSN_FUNC, __FILE__, __LINE__)
  #define dmnsn_unlikely(test)                                    \
    dmnsn_expect(!!(test), false, DMNSN_FUNC, __FILE__, __LINE__)
#elif DMNSN_GNUC
  #define dmnsn_likely(test)   __builtin_expect(!!(test), true)
  #define dmnsn_unlikely(test) __builtin_expect(!!(test), false)
#else
  #define dmnsn_likely(test)   (!!(test))
  #define dmnsn_unlikely(test) (!!(test))
#endif

/**
 * Record a test and its expected result.  Called by dmnsn_[un]likely();
 * don't call directly.
 * @param[in] result    The result of the test.
 * @param[in] expected  The expected result of the test.
 * @param[in] func      The name of the function the test occurs in.
 * @param[in] file      The name of the file the test occurs in.
 * @param[in] line      The line number on which the test occurs.
 * @return \p result.
 */
DMNSN_INTERNAL bool dmnsn_expect(bool result, bool expected, const char *func,
                                 const char *file, unsigned int line);
