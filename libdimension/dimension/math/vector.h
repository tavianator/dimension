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
  double x; /**< The x component. */
  double y; /**< The y component. */
  double z; /**< The z component. */
} dmnsn_vector;

/** A standard format string for vectors. */
#define DMNSN_VECTOR_FORMAT "<%g, %g, %g>"
/** The appropriate arguements to printf() a vector. */
#define DMNSN_VECTOR_PRINTF(v) (v).x, (v).y, (v).z

/* Constants */

/** The zero vector. */
static const dmnsn_vector dmnsn_zero = { 0.0, 0.0, 0.0 };
/** The x vector. */
static const dmnsn_vector dmnsn_x = { 1.0, 0.0, 0.0 };
/** The y vector. */
static const dmnsn_vector dmnsn_y = { 0.0, 1.0, 0.0 };
/** The z vector. */
static const dmnsn_vector dmnsn_z = { 0.0, 0.0, 1.0 };

/* Shorthand for vector construction */

/** Construct a new vector. */
DMNSN_INLINE dmnsn_vector
dmnsn_new_vector(double x, double y, double z)
{
  dmnsn_vector v = { x, y, z };
  return v;
}

/* Vector arithmetic */

/** Negate a vector. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_negate(dmnsn_vector rhs)
{
  /* 3 negations */
  dmnsn_vector v = { -rhs.x, -rhs.y, -rhs.z };
  return v;
}

/** Add two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_add(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 3 additions */
  dmnsn_vector v = { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
  return v;
}

/** Subtract two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_sub(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 3 additions */
  dmnsn_vector v = { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
  return v;
}

/** Multiply a vector by a scalar. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_mul(double lhs, dmnsn_vector rhs)
{
  /* 3 multiplications */
  dmnsn_vector v = { lhs*rhs.x, lhs*rhs.y, lhs*rhs.z };
  return v;
}

/** Divide a vector by a scalar. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_div(dmnsn_vector lhs, double rhs)
{
  /* 3 divisions */
  dmnsn_vector v = { lhs.x/rhs, lhs.y/rhs, lhs.z/rhs };
  return v;
}

/** Return the dot product of two vectors. */
DMNSN_INLINE double
dmnsn_vector_dot(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 3 multiplications, 2 additions */
  return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

/** Return the cross product of two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_cross(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 6 multiplications, 3 additions */
  dmnsn_vector v = { lhs.y*rhs.z - lhs.z*rhs.y,
                     lhs.z*rhs.x - lhs.x*rhs.z,
                     lhs.x*rhs.y - lhs.y*rhs.x };
  return v;
}

/** Return the projection of \p u onto \p d. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_proj(dmnsn_vector u, dmnsn_vector d)
{
  /* 1 division, 9 multiplications, 4 additions */
  return dmnsn_vector_mul(dmnsn_vector_dot(u, d)/dmnsn_vector_dot(d, d), d);
}

/** Return the magnitude of a vector. */
DMNSN_INLINE double
dmnsn_vector_norm(dmnsn_vector n)
{
  /* 1 sqrt, 3 multiplications, 2 additions */
  return sqrt(dmnsn_vector_dot(n, n));
}

/** Return the direction of a vector. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_normalized(dmnsn_vector n)
{
  /* 1 sqrt, 3 divisions, 3 multiplications, 2 additions */
  return dmnsn_vector_div(n, dmnsn_vector_norm(n));
}

/** Return the component-wise minimum of two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_min(dmnsn_vector a, dmnsn_vector b)
{
  return dmnsn_new_vector(
    dmnsn_min(a.x, b.x),
    dmnsn_min(a.y, b.y),
    dmnsn_min(a.z, b.z)
  );
}

/** Return the component-wise maximum of two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_max(dmnsn_vector a, dmnsn_vector b)
{
  return dmnsn_new_vector(
    dmnsn_max(a.x, b.x),
    dmnsn_max(a.y, b.y),
    dmnsn_max(a.z, b.z)
  );
}

/** Return whether a vector contains any NaN components. */
DMNSN_INLINE bool
dmnsn_vector_isnan(dmnsn_vector v)
{
  return dmnsn_isnan(v.x) || dmnsn_isnan(v.y) || dmnsn_isnan(v.z);
}
