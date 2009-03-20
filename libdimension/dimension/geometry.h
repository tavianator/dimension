/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of Dimension.                                       *
 *                                                                       *
 * Dimension is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU Lesser General Public License as published *
 * by the Free Software Foundation; either version 3 of the License, or  *
 * (at your option) any later version.                                   *
 *                                                                       *
 * Dimension is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

/*
 * Core geometric types like scalars, vectors, and rays.
 */

#ifndef DIMENSION_GEOMETRY_H
#define DIMENSION_GEOMETRY_H

/* Scalar and vector types. */
typedef double                           dmnsn_scalar;
typedef struct { dmnsn_scalar x, y, z; } dmnsn_vector;

/* Vector arithmetic */

DMNSN_INLINE dmnsn_vector
dmnsn_vector_construct(dmnsn_scalar x, dmnsn_scalar y, dmnsn_scalar z)
{
  dmnsn_vector v = { .x = x, .y = y, .z = z };
  return v;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_add(dmnsn_vector lhs, dmnsn_vector rhs)
{
  return dmnsn_vector_construct(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_sub(dmnsn_vector lhs, dmnsn_vector rhs)
{
  return dmnsn_vector_construct(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_mul(dmnsn_scalar lhs, dmnsn_vector rhs)
{
  return dmnsn_vector_construct(lhs*rhs.x, lhs*rhs.y, lhs*rhs.z);
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_div(dmnsn_vector lhs, dmnsn_scalar rhs)
{
  return dmnsn_vector_construct(lhs.x/rhs, lhs.y/rhs, lhs.z/rhs);
}

DMNSN_INLINE dmnsn_scalar
dmnsn_vector_dot(dmnsn_vector lhs, dmnsn_vector rhs)
{
  return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_cross(dmnsn_vector lhs, dmnsn_vector rhs)
{
  return dmnsn_vector_construct(lhs.y*rhs.z - lhs.z*rhs.y,
                                lhs.z*rhs.x - lhs.x*rhs.z,
                                lhs.x*rhs.y - lhs.y*rhs.x);
}

/* A line, or ray. */
typedef struct {
  dmnsn_vector x0; /* A point on the line */
  dmnsn_vector n;  /* A normal vector; the direction of the line */
} dmnsn_line;

/* A point on a line, defined by x0 + t*n */
DMNSN_INLINE dmnsn_vector
dmnsn_line_point(dmnsn_line l, dmnsn_scalar t)
{
  return dmnsn_vector_add(l.x0, dmnsn_vector_mul(t, l.n));
}

#endif /* DIMENSION_GEOMETRY_H */
