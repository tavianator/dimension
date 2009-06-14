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
  dmnsn_matrix m = { .n00 = a0, .n01 = a1, .n02 = a2, .n03 = a3,
                     .n10 = b0, .n11 = b1, .n12 = b2, .n13 = b3,
                     .n20 = c0, .n21 = c1, .n22 = c2, .n23 = c3,
                     .n30 = d0, .n31 = d1, .n32 = d2, .n33 = d3 };
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

  axis = dmnsn_vector_normalize(theta);
  angle = dmnsn_vector_norm(theta);

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

/* 4x4 matrix multiplication */
dmnsn_matrix
dmnsn_matrix_mul(dmnsn_matrix lhs, dmnsn_matrix rhs)
{
  dmnsn_matrix r;

  r.n00 = lhs.n00*rhs.n00 + lhs.n01*rhs.n10 + lhs.n02*rhs.n20 + lhs.n03*rhs.n30;
  r.n01 = lhs.n00*rhs.n01 + lhs.n01*rhs.n11 + lhs.n02*rhs.n21 + lhs.n03*rhs.n31;
  r.n02 = lhs.n00*rhs.n02 + lhs.n01*rhs.n12 + lhs.n02*rhs.n22 + lhs.n03*rhs.n32;
  r.n03 = lhs.n00*rhs.n03 + lhs.n01*rhs.n13 + lhs.n02*rhs.n23 + lhs.n03*rhs.n33;

  r.n10 = lhs.n10*rhs.n00 + lhs.n11*rhs.n10 + lhs.n12*rhs.n20 + lhs.n13*rhs.n30;
  r.n11 = lhs.n10*rhs.n01 + lhs.n11*rhs.n11 + lhs.n12*rhs.n21 + lhs.n13*rhs.n31;
  r.n12 = lhs.n10*rhs.n02 + lhs.n11*rhs.n12 + lhs.n12*rhs.n22 + lhs.n13*rhs.n32;
  r.n13 = lhs.n10*rhs.n03 + lhs.n11*rhs.n13 + lhs.n12*rhs.n23 + lhs.n13*rhs.n33;

  r.n20 = lhs.n20*rhs.n00 + lhs.n21*rhs.n10 + lhs.n22*rhs.n20 + lhs.n23*rhs.n30;
  r.n21 = lhs.n20*rhs.n01 + lhs.n21*rhs.n11 + lhs.n22*rhs.n21 + lhs.n23*rhs.n31;
  r.n22 = lhs.n20*rhs.n02 + lhs.n21*rhs.n12 + lhs.n22*rhs.n22 + lhs.n23*rhs.n32;
  r.n23 = lhs.n20*rhs.n03 + lhs.n21*rhs.n13 + lhs.n22*rhs.n23 + lhs.n23*rhs.n33;

  r.n30 = lhs.n30*rhs.n00 + lhs.n31*rhs.n10 + lhs.n32*rhs.n20 + lhs.n33*rhs.n30;
  r.n31 = lhs.n30*rhs.n01 + lhs.n31*rhs.n11 + lhs.n32*rhs.n21 + lhs.n33*rhs.n31;
  r.n32 = lhs.n30*rhs.n02 + lhs.n31*rhs.n12 + lhs.n32*rhs.n22 + lhs.n33*rhs.n32;
  r.n33 = lhs.n30*rhs.n03 + lhs.n31*rhs.n13 + lhs.n32*rhs.n23 + lhs.n33*rhs.n33;

  return r;
}

/* Affine transformation; lhs*(x,y,z,1), normalized so the fourth element is
   1 */
dmnsn_vector
dmnsn_matrix_vector_mul(dmnsn_matrix lhs, dmnsn_vector rhs)
{
  dmnsn_vector r;
  double w;

  r.x = lhs.n00*rhs.x + lhs.n01*rhs.y + lhs.n02*rhs.z + lhs.n03;
  r.y = lhs.n10*rhs.x + lhs.n11*rhs.y + lhs.n12*rhs.z + lhs.n13;
  r.z = lhs.n20*rhs.x + lhs.n21*rhs.y + lhs.n22*rhs.z + lhs.n23;
  w   = lhs.n30*rhs.x + lhs.n31*rhs.y + lhs.n32*rhs.z + lhs.n33;

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
    l.x0);
  return l;
}

/* A point on a line, l.  Returns l.x0 + t*l.n */
dmnsn_vector
dmnsn_line_point(dmnsn_line l, double t)
{
  return dmnsn_vector_add(l.x0, dmnsn_vector_mul(t, l.n));
}
