/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

#ifndef DMNSN_BASE_H
#error "Please include <dimension/base.h> instead of this header directly."
#endif

/**
 * @internal
 * @def DMNSN_C_VERSION
 * The C version according to \p __STDC_VERSION__ if available, otherwise 0.
 */
#ifdef __STDC_VERSION__
  #define DMNSN_C_VERSION __STDC_VERSION__
#else
  #define DMNSN_C_VERSION 0L
#endif

/**
 * @internal
 * @def DMNSN_CXX_VERSION
 * The C++ version according to \p __cplusplus if available, otherwise 0.
 */
#ifdef __cplusplus
  #define DMNSN_CXX_VERSION __cplusplus
#else
  #define DMNSN_CXX_VERSION 0L
#endif

/**
 * @internal
 * Whether we're being compiled as C++.
 */
#define DMNSN_CXX (DMNSN_CXX_VERSION > 0)

/**
 * @internal
 * Whether C++11 features are supported.
 */
#define DMNSN_CXX11 (DMNSN_CXX_VERSION >= 201103L)

/**
 * @internal
 * Whether C99 features are supported.
 */
#define DMNSN_C99 (DMNSN_C_VERSION >= 199901L || DMNSN_CXX11)

/**
 * @internal
 * Whether C11 features are supported.
 */
#define DMNSN_C11 (DMNSN_C_VERSION >= 201112L)

/**
 * @internal
 * Whether GNU C features are supported.
 */
#define DMNSN_GNUC defined(__GNUC__)

/**
 * @def DMNSN_INLINE
 * A portable inline specifier. Expands to the correct method of declaring
 * inline functions for the version of C you are using.
 */
#ifndef DMNSN_INLINE
  #if DMNSN_CXX
    /* C++ inline semantics */
    #define DMNSN_INLINE inline
  #elif DMNSN_C99
    /* C99 inline semantics */
    #define DMNSN_INLINE inline
  #elif DMNSN_GNUC
    /* GCC inline semantics */
    #define DMNSN_INLINE __extension__ extern __inline__
  #else
    /* Unknown C - mark functions static and hope the compiler is smart enough
       to inline them */
    #define DMNSN_INLINE static
  #endif
#endif

/**
 * @def DMNSN_NORETURN
 * A portable noreturn attribute.
 */
#if DMNSN_CXX11
  #define DMNSN_NORETURN [[noreturn]] void
#elif DMNSN_C11
  #define DMNSN_NORETURN _Noreturn void
#elif DMNSN_GNUC
  #define DMNSN_NORETURN __attribute__((noreturn)) void
#else
  #define DMNSN_NORETURN void
#endif

/**
 * @internal
 * @def DMNSN_FUNC
 * @brief Expands to the name of the current function
 */
#if DMNSN_GNUC
  #define DMNSN_FUNC __extension__ __PRETTY_FUNCTION__
#elif DMNSN_C99
  #define DMNSN_FUNC __func__
#else
  #define DMNSN_FUNC "<unknown function>"
#endif

/**
 * @internal
 * An unreachable statement.
 */
#if DMNSN_GNUC
  #define DMNSN_UNREACHABLE() __builtin_unreachable()
#else
  #define DMNSN_UNREACHABLE() ((void)0)
#endif
