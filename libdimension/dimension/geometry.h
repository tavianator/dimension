/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
#include <stdbool.h>

/* Vector and matrix types */

typedef struct dmnsn_vector { double x, y, z; } dmnsn_vector;

typedef struct dmnsn_matrix { double n[4][4]; } dmnsn_matrix;

/* A line, or ray */
typedef struct dmnsn_line {
  dmnsn_vector x0; /* A point on the line */
  dmnsn_vector n;  /* A normal vector; the direction of the line */
} dmnsn_line;

/* A bounding box */
typedef struct dmnsn_bounding_box { dmnsn_vector min, max; } dmnsn_bounding_box;

/* Constants */

#define dmnsn_epsilon 1.0e-9

static const dmnsn_vector dmnsn_zero = { 0.0, 0.0, 0.0 };
static const dmnsn_vector dmnsn_x    = { 1.0, 0.0, 0.0 };
static const dmnsn_vector dmnsn_y    = { 0.0, 1.0, 0.0 };
static const dmnsn_vector dmnsn_z    = { 0.0, 0.0, 1.0 };

/* Shorthand for vector/matrix construction */

DMNSN_INLINE dmnsn_vector
dmnsn_new_vector(double x, double y, double z)
{
  dmnsn_vector v = { x, y, z };
  return v;
}

DMNSN_INLINE dmnsn_matrix
dmnsn_new_matrix(double a0, double a1, double a2, double a3,
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
dmnsn_new_line(dmnsn_vector x0, dmnsn_vector n)
{
  dmnsn_line l = { x0, n };
  return l;
}

/* Vector and matrix arithmetic */

DMNSN_INLINE dmnsn_vector
dmnsn_vector_negate(dmnsn_vector rhs)
{
  /* 3 negations */
  dmnsn_vector v = { -rhs.x, -rhs.y, -rhs.z };
  return v;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_add(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 3 additions */
  dmnsn_vector v = { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
  return v;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_sub(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 3 additions */
  dmnsn_vector v = { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
  return v;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_mul(double lhs, dmnsn_vector rhs)
{
  /* 3 multiplications */
  dmnsn_vector v = { lhs*rhs.x, lhs*rhs.y, lhs*rhs.z };
  return v;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_div(dmnsn_vector lhs, double rhs)
{
  /* 3 divisions */
  dmnsn_vector v = { lhs.x/rhs, lhs.y/rhs, lhs.z/rhs };
  return v;
}

DMNSN_INLINE double
dmnsn_vector_dot(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 3 multiplications, 2 additions */
  return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_cross(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 6 multiplications, 3 additions */
  dmnsn_vector v = { lhs.y*rhs.z - lhs.z*rhs.y,
                     lhs.z*rhs.x - lhs.x*rhs.z,
                     lhs.x*rhs.y - lhs.y*rhs.x };
  return v;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_proj(dmnsn_vector u, dmnsn_vector d)
{
  /* 1 division, 9 multiplications, 4 additions */
  return dmnsn_vector_mul(dmnsn_vector_dot(u, d)/dmnsn_vector_dot(d, d), d);
}

DMNSN_INLINE double
dmnsn_vector_norm(dmnsn_vector n)
{
  /* 1 sqrt, 3 multiplications, 2 additions */
  return sqrt(dmnsn_vector_dot(n, n));
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_normalize(dmnsn_vector n)
{
  /* 1 sqrt, 3 divisions, 3 multiplications, 2 additions */
  return dmnsn_vector_div(n, dmnsn_vector_norm(n));
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_min(dmnsn_vector a, dmnsn_vector b)
{
  dmnsn_vector ret = a;
  if (b.x < ret.x) ret.x = b.x;
  if (b.y < ret.y) ret.y = b.y;
  if (b.z < ret.z) ret.z = b.z;
  return ret;
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_max(dmnsn_vector a, dmnsn_vector b)
{
  dmnsn_vector ret = a;
  if (b.x > ret.x) ret.x = b.x;
  if (b.y > ret.y) ret.y = b.y;
  if (b.z > ret.z) ret.z = b.z;
  return ret;
}

double dmnsn_vector_axis_angle(dmnsn_vector v1, dmnsn_vector v2,
                               dmnsn_vector axis);

dmnsn_matrix dmnsn_matrix_inverse(dmnsn_matrix A);
dmnsn_matrix dmnsn_matrix_mul(dmnsn_matrix lhs, dmnsn_matrix rhs);
dmnsn_vector dmnsn_matrix_vector_mul(dmnsn_matrix lhs, dmnsn_vector rhs);
dmnsn_bounding_box dmnsn_matrix_bounding_box_mul(dmnsn_matrix lhs,
                                                 dmnsn_bounding_box rhs);

/* Affine line transformation; n = lhs*(x0 + n) - lhs*x0, x0 *= lhs */
DMNSN_INLINE dmnsn_line
dmnsn_matrix_line_mul(dmnsn_matrix lhs, dmnsn_line rhs)
{
  /* 24 multiplications, 6 divisions, 30 additions */
  dmnsn_line l;
  l.x0 = dmnsn_matrix_vector_mul(lhs, rhs.x0);
  l.n  = dmnsn_vector_sub(
    dmnsn_matrix_vector_mul(lhs, dmnsn_vector_add(rhs.x0, rhs.n)),
    l.x0
  );
  return l;
}

/* A point on a line, defined by x0 + t*n */
DMNSN_INLINE dmnsn_vector
dmnsn_line_point(dmnsn_line l, double t)
{
  return dmnsn_vector_add(l.x0, dmnsn_vector_mul(t, l.n));
}

/* Add epsilon*l.n to l.x0, to avoid self-intersections */
DMNSN_INLINE dmnsn_line
dmnsn_line_add_epsilon(dmnsn_line l)
{
  return dmnsn_new_line(
    dmnsn_vector_add(
      l.x0,
      dmnsn_vector_mul(dmnsn_epsilon, l.n)
    ),
    l.n
  );
}

/* Solve for the t value such that x0 + t*n = x */
double dmnsn_line_index(dmnsn_line l, dmnsn_vector x);

/* Return whether p is within the axis-aligned bounding box */
DMNSN_INLINE bool
dmnsn_bounding_box_contains(dmnsn_bounding_box box, dmnsn_vector p)
{
  return (p.x >= box.min.x && p.y >= box.min.y && p.z >= box.min.z)
      && (p.x <= box.max.x && p.y <= box.max.y && p.z <= box.max.z);
}

#endif /* DIMENSION_GEOMETRY_H */
