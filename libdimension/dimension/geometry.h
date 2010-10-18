/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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
#define DMNSN_VECTOR_FORMAT "<%g, %g, %g>"
#define DMNSN_VECTOR_PRINTF(v) (v).x, (v).y, (v).z

typedef struct dmnsn_matrix { double n[4][4]; } dmnsn_matrix;
#define DMNSN_MATRIX_FORMAT                     \
  "[%g\t%g\t%g\t%g]\n"                          \
  "[%g\t%g\t%g\t%g]\n"                          \
  "[%g\t%g\t%g\t%g]\n"                          \
  "[%g\t%g\t%g\t%g]"
#define DMNSN_MATRIX_PRINTF(m)                          \
  (m).n[0][0], (m).n[0][1], (m).n[0][2], (m).n[0][3],   \
  (m).n[1][0], (m).n[1][1], (m).n[1][2], (m).n[1][3],   \
  (m).n[2][0], (m).n[2][1], (m).n[2][2], (m).n[2][3],   \
  (m).n[3][0], (m).n[3][1], (m).n[3][2], (m).n[3][3]

/* A line, or ray */
typedef struct dmnsn_line {
  dmnsn_vector x0; /* A point on the line */
  dmnsn_vector n;  /* A normal vector; the direction of the line */
} dmnsn_line;
#define DMNSN_LINE_FORMAT "(<%g, %g, %g> + t*<%g, %g, %g>)"
#define DMNSN_LINE_PRINTF(l)                                    \
  DMNSN_VECTOR_PRINTF((l).x0), DMNSN_VECTOR_PRINTF((l).n)

/* A bounding box */
typedef struct dmnsn_bounding_box { dmnsn_vector min, max; } dmnsn_bounding_box;
#define DMNSN_BOUNDING_BOX_FORMAT "(<%g, %g, %g> ==> <%g, %g, %g>)"
#define DMNSN_BOUNDING_BOX_PRINTF(box)                                  \
  DMNSN_VECTOR_PRINTF((box).min), DMNSN_VECTOR_PRINTF((box).max)

/* Constants */

#define dmnsn_epsilon 1.0e-9

static const dmnsn_vector dmnsn_zero = { 0.0, 0.0, 0.0 };
static const dmnsn_vector dmnsn_x    = { 1.0, 0.0, 0.0 };
static const dmnsn_vector dmnsn_y    = { 0.0, 1.0, 0.0 };
static const dmnsn_vector dmnsn_z    = { 0.0, 0.0, 1.0 };

/* Scalar functions */

DMNSN_INLINE double
dmnsn_min(double a, double b)
{
  return a < b ? a : b;
}

DMNSN_INLINE double
dmnsn_max(double a, double b)
{
  return a > b ? a : b;
}

DMNSN_INLINE double
dmnsn_radians(double degrees)
{
  return degrees*atan(1.0)/45.0;
}

DMNSN_INLINE double
dmnsn_degrees(double radians)
{
  return radians*45.0/atan(1.0);
}

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

dmnsn_matrix dmnsn_identity_matrix(void);
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

DMNSN_INLINE dmnsn_bounding_box
dmnsn_zero_bounding_box(void)
{
  dmnsn_bounding_box box = {
    {  INFINITY,  INFINITY,  INFINITY },
    { -INFINITY, -INFINITY, -INFINITY }
  };
  return box;
}

DMNSN_INLINE dmnsn_bounding_box
dmnsn_infinite_bounding_box(void)
{
  dmnsn_bounding_box box = {
    { -INFINITY, -INFINITY, -INFINITY },
    {  INFINITY,  INFINITY,  INFINITY }
  };
  return box;
}

/* Vector element access */

enum {
  DMNSN_X,
  DMNSN_Y,
  DMNSN_Z
};

DMNSN_INLINE double
dmnsn_vector_element(dmnsn_vector n, int elem)
{
  switch (elem) {
  case DMNSN_X:
    return n.x;
  case DMNSN_Y:
    return n.y;
  case DMNSN_Z:
    return n.z;

  default:
    dmnsn_assert(false, "Wrong vector element requested.");
    return 0.0; /* Shut up compiler */
  }
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
  return dmnsn_new_vector(
    dmnsn_min(a.x, b.x),
    dmnsn_min(a.y, b.y),
    dmnsn_min(a.z, b.z)
  );
}

DMNSN_INLINE dmnsn_vector
dmnsn_vector_max(dmnsn_vector a, dmnsn_vector b)
{
  return dmnsn_new_vector(
    dmnsn_max(a.x, b.x),
    dmnsn_max(a.y, b.y),
    dmnsn_max(a.z, b.z)
  );
}

double dmnsn_vector_axis_angle(dmnsn_vector v1, dmnsn_vector v2,
                               dmnsn_vector axis);

dmnsn_matrix dmnsn_matrix_inverse(dmnsn_matrix A);
dmnsn_matrix dmnsn_matrix_mul(dmnsn_matrix lhs, dmnsn_matrix rhs);

/* Affine transformation; lhs*(x,y,z,1), normalized so the fourth element is
   1 */
DMNSN_INLINE dmnsn_vector
dmnsn_transform_vector(dmnsn_matrix lhs, dmnsn_vector rhs)
{
  /* 12 multiplications, 3 divisions, 12 additions */
  dmnsn_vector r;
  double w;

  r.x = lhs.n[0][0]*rhs.x + lhs.n[0][1]*rhs.y + lhs.n[0][2]*rhs.z + lhs.n[0][3];
  r.y = lhs.n[1][0]*rhs.x + lhs.n[1][1]*rhs.y + lhs.n[1][2]*rhs.z + lhs.n[1][3];
  r.z = lhs.n[2][0]*rhs.x + lhs.n[2][1]*rhs.y + lhs.n[2][2]*rhs.z + lhs.n[2][3];
  w   = lhs.n[3][0]*rhs.x + lhs.n[3][1]*rhs.y + lhs.n[3][2]*rhs.z + lhs.n[3][3];

  return dmnsn_vector_div(r, w);
}

dmnsn_bounding_box dmnsn_transform_bounding_box(dmnsn_matrix lhs,
                                                dmnsn_bounding_box rhs);

/* Affine line transformation; n = lhs*(x0 + n) - lhs*x0, x0 *= lhs */
DMNSN_INLINE dmnsn_line
dmnsn_transform_line(dmnsn_matrix lhs, dmnsn_line rhs)
{
  /* 24 multiplications, 6 divisions, 30 additions */
  dmnsn_line l;
  l.x0 = dmnsn_transform_vector(lhs, rhs.x0);
  l.n  = dmnsn_vector_sub(
    dmnsn_transform_vector(lhs, dmnsn_vector_add(rhs.x0, rhs.n)),
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

/* Return whether `box' is infinite */
DMNSN_INLINE bool
dmnsn_bounding_box_is_infinite(dmnsn_bounding_box box)
{
  return box.min.x == -INFINITY;
}

#endif /* DIMENSION_GEOMETRY_H */
