/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Internally-used compiler abstractions.
 */

#include <stdalign.h>

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
 * @def DMNSN_HOT
 * Mark a function as a hot path.
 */
/**
 * @def DMNSN_INTERNAL
 * Mark a function as internal linkage.
 */
/**
 * @def DMNSN_DESTRUCTOR
 * Queue a function to run at program termination.
 */
/**
 * @def DMNSN_LATE_DESTRUCTOR
 * Queue a function to run at program termination, after those labeled
 * DMNSN_DESTRUCTOR.
 */
#if DMNSN_GNUC
  #define DMNSN_HOT             __attribute__((hot))
  #define DMNSN_INTERNAL        __attribute__((visibility("hidden")))
  #define DMNSN_DESTRUCTOR      __attribute__((destructor(102)))
  #define DMNSN_LATE_DESTRUCTOR __attribute__((destructor(101)))
#else
  #define DMNSN_HOT
  #define DMNSN_INTERNAL
  #define DMNSN_DESTRUCTOR
  #define DMNSN_LATE_DESTRUCTOR
#endif

/// Synonym for _Atomic that stdatomic.h doesn't define for some reason
#define atomic _Atomic

/// C11-compliant alloca variant
#define DMNSN_ALLOCA(var, size) DMNSN_ALLOCA_IMPL(var, size, __LINE__)

#define DMNSN_ALLOCA_IMPL(var, size, ctr) DMNSN_ALLOCA_IMPL2(var, size, ctr)

#define DMNSN_ALLOCA_IMPL2(var, size, ctr)              \
  alignas(max_align_t) char dmnsn_alloca##ctr[size];    \
  var = (void *)dmnsn_alloca##ctr
