/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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

/*
 * Core geometric types like vectors, matricies, and rays.
 */

#ifndef DIMENSION_GEOMETRY_H
#define DIMENSION_GEOMETRY_H

#include <math.h>

/* Vector and matrix types. */

typedef struct { double x, y, z; } dmnsn_vector;

typedef struct { double n[4][4]; } dmnsn_matrix;

/* A line, or ray. */
typedef struct {
  dmnsn_vector x0; /* A point on the line */
  dmnsn_vector n;  /* A normal vector; the direction of the line */
} dmnsn_line;

/* Shorthand for vector/matrix construction */

DMNSN_INLINE dmnsn_vector
dmnsn_vector_construct(double x, double y, double z)
{
  dmnsn_vector v = { x, y, z };
  return v;
}

DMNSN_INLINE dmnsn_matrix
dmnsn_matrix_construct(double a0, double a1, double a2, double a3,
                       double b0, double b1, double b2, double b3,
                       double c0, double c1, double c2, double c3,
                       double d0, double d1, double d2, double d3)
{
  dmnsn_matrix m = { { { a0, a1, a2, a3 },
                       { b0, b1, b2, b3 },
                       { c0, c1, c2, c3 },
                       { d0, d1, d2, d3 } } };
  return m;
}

dmnsn_matrix dmnsn_identity_matrix();
dmnsn_matrix dmnsn_scale_matrix(dmnsn_vector s);
dmnsn_matrix dmnsn_translation_matrix(dmnsn_vector d);
/* Left-handed rotation; theta/|theta| = axis, |theta| = angle */
dmnsn_matrix dmnsn_rotation_matrix(dmnsn_vector theta);

DMNSN_INLINE dmnsn_line
dmnsn_line_construct(dmnsn_vector x0, dmnsn_vector n)
{
  dmnsn_line l = { x0, n };
  return l;
}

/* Vector and matrix arithmetic */

DMNSN_INLINE dmnsn_vector
dmnsn_vector_add(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
  return v;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_sub(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
  return v;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_mul(double lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { lhs*rhs.x, lhs*rhs.y, lhs*rhs.z };
  return v;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_div(dmnsn_vector lhs, double rhs)
{
  dmnsn_vector v = { lhs.x/rhs, lhs.y/rhs, lhs.z/rhs };
  return v;
}

DMNSN_INLINE double
dmnsn_vector_dot(dmnsn_vector lhs, dmnsn_vector rhs)
{
  return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_cross(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { lhs.y*rhs.z - lhs.z*rhs.y,
                     lhs.z*rhs.x - lhs.x*rhs.z,
                     lhs.x*rhs.y - lhs.y*rhs.x };
  return v;
}

DMNSN_INLINE double
dmnsn_vector_norm(dmnsn_vector n)
{
  return sqrt(dmnsn_vector_dot(n, n));
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_normalize(dmnsn_vector n)
{
  return dmnsn_vector_div(n, dmnsn_vector_norm(n));
}

dmnsn_matrix dmnsn_matrix_inverse(dmnsn_matrix A);
dmnsn_matrix dmnsn_matrix_mul(dmnsn_matrix lhs, dmnsn_matrix rhs);
dmnsn_vector dmnsn_matrix_vector_mul(dmnsn_matrix lhs, dmnsn_vector rhs);
dmnsn_line   dmnsn_matrix_line_mul(dmnsn_matrix lhs, dmnsn_line rhs);

/* A point on a line, defined by x0 + t*n */
DMNSN_INLINE dmnsn_vector
dmnsn_line_point(dmnsn_line l, double t)
{
  return dmnsn_vector_add(l.x0, dmnsn_vector_mul(t, l.n));
}

/* Solve for the t value such that x0 + t*n = x */
double dmnsn_line_index(dmnsn_line l, dmnsn_vector x);

#endif /* DIMENSION_GEOMETRY_H */
