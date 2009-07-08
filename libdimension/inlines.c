/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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

/* Set DMNSN_INLINE to produce definitions of inline functions, emitted here,
   if needed */
#ifdef __cplusplus
  /* C++ inline semantics */
  #define DMNSN_INLINE inline
#elif (__STDC_VERSION__ >= 199901L)
  /* C99 inline semantics */
  #define DMNSN_INLINE
#elif defined(__GNUC__)
  /* GCC inline semantics */
  #define DMNSN_INLINE __inline__
#else
  /* Unknown C - mark functions static and hope the compiler is smart enough
     to inline them */
  #define DMNSN_INLINE static
#endif

#include "dimension.h"
