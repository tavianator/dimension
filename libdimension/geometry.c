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

#include "dimension.h"
#include <math.h>

/* Construct a vector from x, y, and z.  Just for convienence. */
dmnsn_vector
dmnsn_vector_construct(double x, double y, double z)
{
  dmnsn_vector v = { .x = x, .y = y, .z = z };
  return v;
}

/* Construct a matrix. */
dmnsn_matrix
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

/* Identity matrix */
dmnsn_matrix
dmnsn_identity_matrix()
{
  return dmnsn_matrix_construct(1.0, 0.0, 0.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                0.0, 0.0, 0.0, 1.0);
}

/* Scaling matrix */
dmnsn_matrix
dmnsn_scale_matrix(dmnsn_vector s)
{
  return dmnsn_matrix_construct(s.x, 0.0, 0.0, 0.0,
                                0.0, s.y, 0.0, 0.0,
                                0.0, 0.0, s.z, 0.0,
                                0.0, 0.0, 0.0, 1.0);
}

/* Translation matrix */
dmnsn_matrix
dmnsn_translation_matrix(dmnsn_vector d)
{
  return dmnsn_matrix_construct(1.0, 0.0, 0.0, d.x,
                                0.0, 1.0, 0.0, d.y,
                                0.0, 0.0, 1.0, d.z,
                                0.0, 0.0, 0.0, 1.0);
}

/* Left-handed rotation matrix; theta/|theta| = axis, |theta| = angle */
dmnsn_matrix
dmnsn_rotation_matrix(dmnsn_vector theta)
{
  dmnsn_vector axis, n1, n2, n3;
  double angle, s, t, x, y, z;

  angle = dmnsn_vector_norm(theta);
  if (angle != 0.0) {
    axis = dmnsn_vector_normalize(theta);

    /* Shorthand to fit logical lines on one line */

    s = sin(angle);
    t = 1.0 - cos(angle);

    x = axis.x;
    y = axis.y;
    z = axis.z;

    /* Construct vectors, then a matrix, so our dmnsn_matrix_construct() call
       is reasonably small */

    n1 = dmnsn_vector_construct(1.0 + t*(x*x - 1.0),
                                z*s + t*x*y,
                                -y*s + t*x*z);
    n2 = dmnsn_vector_construct(-z*s + t*x*y,
                                1.0 + t*(y*y - 1.0),
                                x*s + t*y*z);
    n3 = dmnsn_vector_construct(y*s + t*x*z,
                                -x*s + t*y*z,
                                1.0 + t*(z*z - 1.0));

    return dmnsn_matrix_construct(n1.x, n2.x, n3.x, 0.0,
                                  n1.y, n2.y, n3.y, 0.0,
                                  n1.z, n2.z, n3.z, 0.0,
                                  0.0,  0.0,  0.0,  1.0);
  } else {
    return dmnsn_identity_matrix();
  }
}

/* Add two vectors */
dmnsn_vector
dmnsn_vector_add(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { .x = lhs.x + rhs.x,
                     .y = lhs.y + rhs.y,
                     .z = lhs.z + rhs.z };
  return v;
}

/* Subtract two vectors */
dmnsn_vector
dmnsn_vector_sub(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { .x = lhs.x - rhs.x,
                     .y = lhs.y - rhs.y,
                     .z = lhs.z - rhs.z };
  return v;
}

/* Multiply a vector by a scalar */
dmnsn_vector
dmnsn_vector_mul(double lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { .x = lhs*rhs.x, .y = lhs*rhs.y, .z = lhs*rhs.z };
  return v;
}

/* Divide a vector by a scalar */
dmnsn_vector
dmnsn_vector_div(dmnsn_vector lhs, double rhs)
{
  dmnsn_vector v = { .x = lhs.x/rhs, .y = lhs.y/rhs, .z = lhs.z/rhs };
  return v;
}

/* Dot product */
double
dmnsn_vector_dot(dmnsn_vector lhs, dmnsn_vector rhs)
{
  return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

/* Cross product */
dmnsn_vector
dmnsn_vector_cross(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { .x = lhs.y*rhs.z - lhs.z*rhs.y,
                     .y = lhs.z*rhs.x - lhs.x*rhs.z,
                     .z = lhs.x*rhs.y - lhs.y*rhs.x };
  return v;
}

/* Length of vector */
double
dmnsn_vector_norm(dmnsn_vector n)
{
  return sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
}

/* Normalized vector */
dmnsn_vector
dmnsn_vector_normalize(dmnsn_vector n)
{
  return dmnsn_vector_div(n, dmnsn_vector_norm(n));
}

/* Matrix inversion helper function */
static double dmnsn_matrix_cofactor(dmnsn_matrix A,
                                    unsigned int row, unsigned int col);

/* Invert a matrix */
dmnsn_matrix
dmnsn_matrix_inverse(dmnsn_matrix A)
{
  dmnsn_matrix adj;
  double det = 0.0, C;
  unsigned int i, j;

  for (j = 0; j < 4; ++j) {
    C = dmnsn_matrix_cofactor(A, 0, j);
    det += A.n[0][j]*C;
    adj.n[j][0] = C;
  }

  for (j = 0; j < 4; ++j) {
    adj.n[j][0] /= det;
  }

  for (i = 1; i < 4; ++i) {
    for (j = 0; j < 4; ++j) {
      adj.n[j][i] = dmnsn_matrix_cofactor(A, i, j)/det;
    }
  }

  return adj;
}

/* Gives the cofactor at row, col; the determinant of the matrix formed from A
   by ignoring row `row' and column `col', times (-1)**(row + col) */
static double
dmnsn_matrix_cofactor(dmnsn_matrix A, unsigned int row, unsigned int col)
{
  double n[9], C;
  unsigned int i, j, k = 0;

  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j) {
      if (i != row && j != col) {
        n[k] = A.n[i][j];
        ++k;
      }
    }
  }

  C = n[0]*(n[4]*n[8] - n[5]*n[7]) + n[1]*(n[5]*n[6] - n[3]*n[8])
    + n[2]*(n[3]*n[7] - n[4]*n[6]);
  if ((row + col)%2 == 0) {
    return C;
  } else {
    return -C;
  }
}

/* 4x4 matrix multiplication */
dmnsn_matrix
dmnsn_matrix_mul(dmnsn_matrix lhs, dmnsn_matrix rhs)
{
  dmnsn_matrix r;

  r.n[0][0] = lhs.n[0][0]*rhs.n[0][0] + lhs.n[0][1]*rhs.n[1][0]
    + lhs.n[0][2]*rhs.n[2][0] + lhs.n[0][3]*rhs.n[3][0];
  r.n[0][1] = lhs.n[0][0]*rhs.n[0][1] + lhs.n[0][1]*rhs.n[1][1]
    + lhs.n[0][2]*rhs.n[2][1] + lhs.n[0][3]*rhs.n[3][1];
  r.n[0][2] = lhs.n[0][0]*rhs.n[0][2] + lhs.n[0][1]*rhs.n[1][2]
    + lhs.n[0][2]*rhs.n[2][2] + lhs.n[0][3]*rhs.n[3][2];
  r.n[0][3] = lhs.n[0][0]*rhs.n[0][3] + lhs.n[0][1]*rhs.n[1][3]
    + lhs.n[0][2]*rhs.n[2][3] + lhs.n[0][3]*rhs.n[3][3];

  r.n[1][0] = lhs.n[1][0]*rhs.n[0][0] + lhs.n[1][1]*rhs.n[1][0]
    + lhs.n[1][2]*rhs.n[2][0] + lhs.n[1][3]*rhs.n[3][0];
  r.n[1][1] = lhs.n[1][0]*rhs.n[0][1] + lhs.n[1][1]*rhs.n[1][1]
    + lhs.n[1][2]*rhs.n[2][1] + lhs.n[1][3]*rhs.n[3][1];
  r.n[1][2] = lhs.n[1][0]*rhs.n[0][2] + lhs.n[1][1]*rhs.n[1][2]
    + lhs.n[1][2]*rhs.n[2][2] + lhs.n[1][3]*rhs.n[3][2];
  r.n[1][3] = lhs.n[1][0]*rhs.n[0][3] + lhs.n[1][1]*rhs.n[1][3]
    + lhs.n[1][2]*rhs.n[2][3] + lhs.n[1][3]*rhs.n[3][3];

  r.n[2][0] = lhs.n[2][0]*rhs.n[0][0] + lhs.n[2][1]*rhs.n[1][0]
    + lhs.n[2][2]*rhs.n[2][0] + lhs.n[2][3]*rhs.n[3][0];
  r.n[2][1] = lhs.n[2][0]*rhs.n[0][1] + lhs.n[2][1]*rhs.n[1][1]
    + lhs.n[2][2]*rhs.n[2][1] + lhs.n[2][3]*rhs.n[3][1];
  r.n[2][2] = lhs.n[2][0]*rhs.n[0][2] + lhs.n[2][1]*rhs.n[1][2]
    + lhs.n[2][2]*rhs.n[2][2] + lhs.n[2][3]*rhs.n[3][2];
  r.n[2][3] = lhs.n[2][0]*rhs.n[0][3] + lhs.n[2][1]*rhs.n[1][3]
    + lhs.n[2][2]*rhs.n[2][3] + lhs.n[2][3]*rhs.n[3][3];

  r.n[3][0] = lhs.n[3][0]*rhs.n[0][0] + lhs.n[3][1]*rhs.n[1][0]
    + lhs.n[3][2]*rhs.n[2][0] + lhs.n[3][3]*rhs.n[3][0];
  r.n[3][1] = lhs.n[3][0]*rhs.n[0][1] + lhs.n[3][1]*rhs.n[1][1]
    + lhs.n[3][2]*rhs.n[2][1] + lhs.n[3][3]*rhs.n[3][1];
  r.n[3][2] = lhs.n[3][0]*rhs.n[0][2] + lhs.n[3][1]*rhs.n[1][2]
    + lhs.n[3][2]*rhs.n[2][2] + lhs.n[3][3]*rhs.n[3][2];
  r.n[3][3] = lhs.n[3][0]*rhs.n[0][3] + lhs.n[3][1]*rhs.n[1][3]
    + lhs.n[3][2]*rhs.n[2][3] + lhs.n[3][3]*rhs.n[3][3];

  return r;
}

/* Affine transformation; lhs*(x,y,z,1), normalized so the fourth element is
   1 */
dmnsn_vector
dmnsn_matrix_vector_mul(dmnsn_matrix lhs, dmnsn_vector rhs)
{
  dmnsn_vector r;
  double w;

  r.x = lhs.n[0][0]*rhs.x + lhs.n[0][1]*rhs.y + lhs.n[0][2]*rhs.z + lhs.n[0][3];
  r.y = lhs.n[1][0]*rhs.x + lhs.n[1][1]*rhs.y + lhs.n[1][2]*rhs.z + lhs.n[1][3];
  r.z = lhs.n[2][0]*rhs.x + lhs.n[2][1]*rhs.y + lhs.n[2][2]*rhs.z + lhs.n[2][3];
  w   = lhs.n[3][0]*rhs.x + lhs.n[3][1]*rhs.y + lhs.n[3][2]*rhs.z + lhs.n[3][3];

  return dmnsn_vector_div(r, w);
}

/* Affine line transformation; n = lhs*(x0 + n) - lhs*x0, x0 *= lhs */
dmnsn_line
dmnsn_matrix_line_mul(dmnsn_matrix lhs, dmnsn_line rhs)
{
  dmnsn_line l;
  l.x0 = dmnsn_matrix_vector_mul(lhs, rhs.x0);
  l.n  = dmnsn_vector_sub(
    dmnsn_matrix_vector_mul(lhs, dmnsn_vector_add(rhs.x0, rhs.n)),
    l.x0
  );
  return l;
}

/* A point on a line, l.  Returns l.x0 + t*l.n */
dmnsn_vector
dmnsn_line_point(dmnsn_line l, double t)
{
  return dmnsn_vector_add(l.x0, dmnsn_vector_mul(t, l.n));
}

/* Solve for the t value such that x0 + t*n = x */
double
dmnsn_line_index(dmnsn_line l, dmnsn_vector x)
{
  double d = 0.0;
  unsigned int nz = 0;

  if (l.n.x != 0.0) {
    d += (x.x - l.x0.x)/l.n.x;
    ++nz;
  }

  if (l.n.y != 0.0) {
    d += (x.y - l.x0.y)/l.n.y;
    ++nz;
  }

  if (l.n.z != 0.0) {
    d += (x.z - l.x0.z)/l.n.z;
    ++nz;
  }

  return d/nz;
}
