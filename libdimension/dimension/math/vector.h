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
 * Vectors in 3-D space.
 */

#ifndef DMNSN_MATH_H
#error "Please include <dimension/math.h> instead of this header directly."
#endif

#include <math.h>
#include <stdbool.h>

/** A vector in 3 dimensions. */
typedef struct dmnsn_vector {
  double n[3]; /**< The components. */
} dmnsn_vector;

/** A standard format string for vectors. */
#define DMNSN_VECTOR_FORMAT "<%g, %g, %g>"
/** The appropriate arguements to printf() a vector. */
#define DMNSN_VECTOR_PRINTF(v) (v).n[0], (v).n[1], (v).n[2]

/* Constants */

/** The zero vector. */
static const dmnsn_vector dmnsn_zero = { { 0.0, 0.0, 0.0 } };
/** The x vector. */
static const dmnsn_vector dmnsn_x = { { 1.0, 0.0, 0.0 } };
/** The y vector. */
static const dmnsn_vector dmnsn_y = { { 0.0, 1.0, 0.0 } };
/** The z vector. */
static const dmnsn_vector dmnsn_z = { { 0.0, 0.0, 1.0 } };

/* Shorthand for vector construction */

/** Construct a new vector. */
DMNSN_INLINE dmnsn_vector
dmnsn_new_vector(double x, double y, double z)
{
  dmnsn_vector v = { { x, y, z } };
  return v;
}

/* Vector arithmetic */

/** Negate a vector. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_negate(dmnsn_vector rhs)
{
  dmnsn_vector v;
  unsigned int i;
  for (i = 0; i < 3; ++i) {
    v.n[i] = -rhs.n[i];
  }
  return v;
}

/** Add two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_add(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v;
  unsigned int i;
  for (i = 0; i < 3; ++i) {
    v.n[i] = lhs.n[i] + rhs.n[i];
  }
  return v;
}

/** Subtract two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_sub(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v;
  unsigned int i;
  for (i = 0; i < 3; ++i) {
    v.n[i] = lhs.n[i] - rhs.n[i];
  }
  return v;
}

/** Multiply a vector by a scalar. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_mul(double lhs, dmnsn_vector rhs)
{
  dmnsn_vector v;
  unsigned int i;
  for (i = 0; i < 3; ++i) {
    v.n[i] = lhs*rhs.n[i];
  }
  return v;
}

/** Return the dot product of two vectors. */
DMNSN_INLINE double
dmnsn_vector_dot(dmnsn_vector lhs, dmnsn_vector rhs)
{
  double result = 0.0;
  unsigned int i;
  for (i = 0; i < 3; ++i) {
    result += lhs.n[i]*rhs.n[i];
  }
  return result;
}

/** Return the cross product of two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_cross(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v;
  v.n[0] = lhs.n[1]*rhs.n[2] - lhs.n[2]*rhs.n[1];
  v.n[1] = lhs.n[2]*rhs.n[0] - lhs.n[0]*rhs.n[2];
  v.n[2] = lhs.n[0]*rhs.n[1] - lhs.n[1]*rhs.n[0];
  return v;
}

/** Return the projection of \p u onto \p d. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_proj(dmnsn_vector u, dmnsn_vector d)
{
  return dmnsn_vector_mul(dmnsn_vector_dot(u, d)/dmnsn_vector_dot(d, d), d);
}

/** Return the magnitude of a vector. */
DMNSN_INLINE double
dmnsn_vector_norm(dmnsn_vector n)
{
  return sqrt(dmnsn_vector_dot(n, n));
}

/** Return the direction of a vector. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_normalized(dmnsn_vector n)
{
  return dmnsn_vector_mul(1.0/dmnsn_vector_norm(n), n);
}

/** Return the component-wise minimum of two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_min(dmnsn_vector a, dmnsn_vector b)
{
  dmnsn_vector v;
  unsigned int i;
  for (i = 0; i < 3; ++i) {
    v.n[i] = dmnsn_min(a.n[i], b.n[i]);
  }
  return v;
}

/** Return the component-wise maximum of two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_max(dmnsn_vector a, dmnsn_vector b)
{
  dmnsn_vector v;
  unsigned int i;
  for (i = 0; i < 3; ++i) {
    v.n[i] = dmnsn_max(a.n[i], b.n[i]);
  }
  return v;
}

/** Return whether a vector contains any NaN components. */
DMNSN_INLINE bool
dmnsn_vector_isnan(dmnsn_vector v)
{
  unsigned int i;
  for (i = 0; i < 3; ++i) {
    if (dmnsn_isnan(v.n[i])) {
      return true;
    }
  }
  return false;
}
