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

#ifndef DMNSN_INTERNAL_H
#error "Please include \"internal.h\" instead of this header directly."
#endif

#include <stdalign.h>

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

/// Nearly-compliant alloca variant
#define DMNSN_ALLOCA(var, size) DMNSN_ALLOCA_IMPL(var, size, __LINE__)

#define DMNSN_ALLOCA_IMPL(var, size, ctr) DMNSN_ALLOCA_IMPL2(var, size, ctr)

#define DMNSN_ALLOCA_IMPL2(var, size, ctr)              \
  alignas(max_align_t) char dmnsn_alloca##ctr[size];    \
  var = (void *)dmnsn_alloca##ctr
