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
 * Mathematical functions of one variable.
 */

#ifndef DMNSN_MATH_H
#error "Please include <dimension/math.h> instead of this header directly."
#endif

#include <math.h>
#include <stdbool.h>

/** The smallest value considered non-zero by some numerical algorithms. */
#define dmnsn_epsilon 1.0e-10

/**
 * @def DMNSN_INFINITY
 * Expands to floating-point infinity.
 */
#if defined(INFINITY) || DMNSN_C99
  #define DMNSN_INFINITY INFINITY
#else
  #define DMNSN_INFINITY HUGE_VAL
#endif

/** Find the minimum of two values. */
DMNSN_INLINE double
dmnsn_min(double a, double b)
{
  return a < b ? a : b;
}

/** Find the maximum of two values. */
DMNSN_INLINE double
dmnsn_max(double a, double b)
{
  return a > b ? a : b;
}

/** Clamp a value to an interval. */
DMNSN_INLINE double
dmnsn_clamp(double n, double min, double max)
{
  return dmnsn_min(dmnsn_max(n, min), max);
}

/** Convert degrees to radians. */
DMNSN_INLINE double
dmnsn_radians(double degrees)
{
  return degrees*(atan(1.0)/45.0);
}

/** Convert radians to degrees. */
DMNSN_INLINE double
dmnsn_degrees(double radians)
{
  return radians*(45.0/atan(1.0));
}

/** Signum function: return the sign of a value. */
DMNSN_INLINE int
dmnsn_sgn(double n)
{
  if (n > 0.0) {
    return 1;
  } else if (n < 0.0) {
    return -1;
  } else {
    return 0;
  }
}

/** Return whether a value is NaN. */
DMNSN_INLINE bool
dmnsn_isnan(double n)
{
#if DMNSN_C99
  return isnan(n);
#else
  return n != n;
#endif
}
