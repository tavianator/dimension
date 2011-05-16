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
 * Compiler abstractions.
 */

#include <stdbool.h>

#ifdef DMNSN_PROFILE
  #define dmnsn_likely(test)                                      \
    dmnsn_expect(!!(test), true, DMNSN_FUNC, __FILE__, __LINE__)
  #define dmnsn_unlikely(test)                                    \
    dmnsn_expect(!!(test), false, DMNSN_FUNC, __FILE__, __LINE__)
#elif defined(__GNUC__)
  #define dmnsn_likely(test)   __builtin_expect(!!(test), true)
  #define dmnsn_unlikely(test) __builtin_expect(!!(test), false)
#else
  #define dmnsn_likely(test)   (!!(test))
  #define dmnsn_unlikely(test) (!!(test))
#endif

#ifdef __GNUC__
  #define DMNSN_HOT      __attribute__((hot))
  #define DMNSN_INTERNAL __attribute__((visibility("hidden")))
#else
  #define DMNSN_HOT
  #define DMNSN_INTERNAL
#endif
