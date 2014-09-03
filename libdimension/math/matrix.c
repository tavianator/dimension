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
 * Matrix function implementations.
 */

#include "internal.h"
#include "dimension/math.h"
#include <math.h>

// Left-handed rotation matrix; theta/|theta| = axis, |theta| = angle
dmnsn_matrix
dmnsn_rotation_matrix(dmnsn_vector theta)
{
  double angle = dmnsn_vector_norm(theta);
  if (fabs(angle) < dmnsn_epsilon) {
    return dmnsn_identity_matrix();
  }
  dmnsn_vector axis = dmnsn_vector_mul(1.0/angle, theta);

  // Shorthand to make dmnsn_new_matrix() call legible
  double s = sin(angle);
  double t = 1.0 - cos(angle);
  double x = axis.X;
  double y = axis.Y;
  double z = axis.Z;

  return dmnsn_new_matrix(
    1.0 + t*(x*x - 1.0), -z*s + t*x*y,        y*s + t*x*z,         0.0,
    z*s + t*x*y,         1.0 + t*(y*y - 1.0), -x*s + t*y*z,        0.0,
    -y*s + t*x*z,        x*s + t*y*z,         1.0 + t*(z*z - 1.0), 0.0
  );
}

// Find the angle between two vectors with respect to an axis
static double
dmnsn_axis_angle(dmnsn_vector from, dmnsn_vector to, dmnsn_vector axis)
{
  from = dmnsn_vector_sub(from, dmnsn_vector_proj(from, axis));
  to = dmnsn_vector_sub(to, dmnsn_vector_proj(to, axis));

  double fromnorm = dmnsn_vector_norm(from);
  double tonorm   = dmnsn_vector_norm(to);
  if (fromnorm < dmnsn_epsilon || tonorm < dmnsn_epsilon) {
    return 0.0;
  }

  from = dmnsn_vector_mul(1.0/fromnorm, from);
  to = dmnsn_vector_mul(1.0/tonorm, to);

  double angle = acos(dmnsn_vector_dot(from, to));

  if (dmnsn_vector_dot(dmnsn_vector_cross(from, to), axis) > 0.0) {
    return angle;
  } else {
    return -angle;
  }
}

// Alignment matrix
dmnsn_matrix
dmnsn_alignment_matrix(dmnsn_vector from, dmnsn_vector to, dmnsn_vector axis1, dmnsn_vector axis2)
{
  double theta1 = dmnsn_axis_angle(from, to, axis1);
  dmnsn_matrix align1 = dmnsn_rotation_matrix(dmnsn_vector_mul(theta1, axis1));
  from  = dmnsn_transform_direction(align1, from);
  axis2 = dmnsn_transform_direction(align1, axis2);

  double theta2 = dmnsn_axis_angle(from, to, axis2);
  dmnsn_matrix align2 = dmnsn_rotation_matrix(dmnsn_vector_mul(theta2, axis2));

  return dmnsn_matrix_mul(align2, align1);
}

// Matrix inversion helper functions

/// A 2x2 matrix for inversion by partitioning.
typedef struct { double n[2][2]; } dmnsn_matrix2;

/// Construct a 2x2 matrix.
static dmnsn_matrix2 dmnsn_new_matrix2(double a1, double a2,
                                       double b1, double b2);
/// Invert a 2x2 matrix.
static dmnsn_matrix2 dmnsn_matrix2_inverse(dmnsn_matrix2 A);
/// Negate a 2x2 matrix.
static dmnsn_matrix2 dmnsn_matrix2_negate(dmnsn_matrix2 A);
/// Subtract two 2x2 matricies.
static dmnsn_matrix2 dmnsn_matrix2_sub(dmnsn_matrix2 lhs, dmnsn_matrix2 rhs);
/// Add two 2x2 matricies.
static dmnsn_matrix2 dmnsn_matrix2_mul(dmnsn_matrix2 lhs, dmnsn_matrix2 rhs);

/// Invert a matrix with the slower cofactor algorithm, if partitioning failed.
static dmnsn_matrix dmnsn_matrix_inverse_generic(const dmnsn_matrix M);
/// Get the [\p row, \p col] cofactor of A.
static double dmnsn_matrix_cofactor(dmnsn_matrix M, size_t row, size_t col);

// Invert a matrix, by partitioning
dmnsn_matrix
dmnsn_matrix_inverse(dmnsn_matrix M)
{
  // Use partitioning to invert a matrix:
  //
  //     [ P Q ] -1
  //     [ R S ]
  //
  //   = [ PP QQ ]
  //     [ RR SS ],
  //
  // with PP = inv(P) - inv(P)*Q*RR,
  //      QQ = -inv(P)*Q*SS,
  //      RR = -SS*R*inv(P), and
  //      SS = inv(S - R*inv(P)*Q).

  dmnsn_matrix2 P, Q, R, S, Pi, RPi, PiQ, RPiQ, PP, QQ, RR, SS;
  double Pdet = M.n[0][0]*M.n[1][1] - M.n[0][1]*M.n[1][0];

  if (dmnsn_unlikely(fabs(Pdet) < dmnsn_epsilon)) {
    // If P is close to singular, try a more generic algorithm; this is very
    // unlikely, but not impossible, eg.
    //   [ 1 1 0 0 ]
    //   [ 1 1 1 0 ]
    //   [ 0 1 1 0 ]
    //   [ 0 0 0 1 ]
    return dmnsn_matrix_inverse_generic(M);
  }

  double Pdet_inv = 1.0/Pdet;

  // Partition the matrix
  P = dmnsn_new_matrix2(M.n[0][0], M.n[0][1],
                        M.n[1][0], M.n[1][1]);
  Q = dmnsn_new_matrix2(M.n[0][2], M.n[0][3],
                        M.n[1][2], M.n[1][3]);
  R = dmnsn_new_matrix2(M.n[2][0], M.n[2][1],
                        0.0,       0.0);
  S = dmnsn_new_matrix2(M.n[2][2], M.n[2][3],
                        0.0,       1.0);

  // Do this inversion ourselves, since we already have the determinant
  Pi = dmnsn_new_matrix2( P.n[1][1]*Pdet_inv, -P.n[0][1]*Pdet_inv,
                         -P.n[1][0]*Pdet_inv,  P.n[0][0]*Pdet_inv);

  // Calculate R*inv(P), inv(P)*Q, and R*inv(P)*Q
  RPi  = dmnsn_matrix2_mul(R, Pi);
  PiQ  = dmnsn_matrix2_mul(Pi, Q);
  RPiQ = dmnsn_matrix2_mul(R, PiQ);

  // Calculate the partitioned inverse
  SS = dmnsn_matrix2_inverse(dmnsn_matrix2_sub(S, RPiQ));
  RR = dmnsn_matrix2_negate(dmnsn_matrix2_mul(SS, RPi));
  QQ = dmnsn_matrix2_negate(dmnsn_matrix2_mul(PiQ, SS));
  PP = dmnsn_matrix2_sub(Pi, dmnsn_matrix2_mul(PiQ, RR));

  // Reconstruct the matrix
  return dmnsn_new_matrix(
    PP.n[0][0], PP.n[0][1], QQ.n[0][0], QQ.n[0][1],
    PP.n[1][0], PP.n[1][1], QQ.n[1][0], QQ.n[1][1],
    RR.n[0][0], RR.n[0][1], SS.n[0][0], SS.n[0][1]
  );
}

// For nice shorthand
static dmnsn_matrix2
dmnsn_new_matrix2(double a1, double a2, double b1, double b2)
{
  dmnsn_matrix2 m = { { { a1, a2 },
                        { b1, b2 } } };
  return m;
}

// Invert a 2x2 matrix
static dmnsn_matrix2
dmnsn_matrix2_inverse(dmnsn_matrix2 A)
{
  double inv_det = 1.0/(A.n[0][0]*A.n[1][1] - A.n[0][1]*A.n[1][0]);
  return dmnsn_new_matrix2( A.n[1][1]*inv_det, -A.n[0][1]*inv_det,
                           -A.n[1][0]*inv_det,  A.n[0][0]*inv_det);
}

// Also basically a shorthand
static dmnsn_matrix2
dmnsn_matrix2_negate(dmnsn_matrix2 A)
{
  return dmnsn_new_matrix2(-A.n[0][0], -A.n[0][1],
                           -A.n[1][0], -A.n[1][1]);
}

// 2x2 matrix subtraction
static dmnsn_matrix2
dmnsn_matrix2_sub(dmnsn_matrix2 lhs, dmnsn_matrix2 rhs)
{
  return dmnsn_new_matrix2(
    lhs.n[0][0] - rhs.n[0][0], lhs.n[0][1] - rhs.n[0][1],
    lhs.n[1][0] - rhs.n[1][0], lhs.n[1][1] - rhs.n[1][1]
  );
}

// 2x2 matrix multiplication
static dmnsn_matrix2
dmnsn_matrix2_mul(dmnsn_matrix2 lhs, dmnsn_matrix2 rhs)
{
  return dmnsn_new_matrix2(
    lhs.n[0][0]*rhs.n[0][0] + lhs.n[0][1]*rhs.n[1][0],
      lhs.n[0][0]*rhs.n[0][1] + lhs.n[0][1]*rhs.n[1][1],
    lhs.n[1][0]*rhs.n[0][0] + lhs.n[1][1]*rhs.n[1][0],
      lhs.n[1][0]*rhs.n[0][1] + lhs.n[1][1]*rhs.n[1][1]
  );
}

// Invert a matrix, if partitioning failed (|P| == 0)
static dmnsn_matrix
dmnsn_matrix_inverse_generic(dmnsn_matrix M)
{
  // For A = [ A'      b ]  A^-1 = [ A'^-1   -(A'^-1)*b ]
  //         [ 0 ... 0 1 ],        [ 0 ... 0      1     ].
  //
  // Invert A' by calculating its adjucate.
  dmnsn_matrix inv;
  double det = 0.0, C;

  // Perform a Laplace expansion along the first row to give us the adjugate's
  // first column and the determinant
  for (size_t j = 0; j < 3; ++j) {
    C = dmnsn_matrix_cofactor(M, 0, j);
    det += M.n[0][j]*C;
    inv.n[j][0] = C;
  }

  double inv_det = 1.0/det;

  // Divide the first column by the determinant
  for (size_t j = 0; j < 3; ++j) {
    inv.n[j][0] *= inv_det;
  }

  // Find the rest of A'
  for (size_t j = 0; j < 3; ++j) {
    for (size_t i = 1; i < 3; ++i) {
      inv.n[j][i] = dmnsn_matrix_cofactor(M, i, j)*inv_det;
    }
    inv.n[j][3] = 0.0;
  }

  // Find the translational component of the inverse
  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 0; j < 3; ++j) {
      inv.n[i][3] -= inv.n[i][j]*M.n[j][3];
    }
  }

  return inv;
}

// Gives the cofactor at row, col; the determinant of the matrix formed from the
// upper-left 3x3 corner of A by ignoring row `row' and column `col',
// times (-1)^(row + col)
static double
dmnsn_matrix_cofactor(dmnsn_matrix M, size_t row, size_t col)
{
  double n[4];
  size_t k = 0;
  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 0; j < 3; ++j) {
      if (i != row && j != col) {
        n[k] = M.n[i][j];
        ++k;
      }
    }
  }

  double C = n[0]*n[3] - n[1]*n[2];
  if ((row + col)%2 == 0) {
    return C;
  } else {
    return -C;
  }
}

// Give an axis-aligned box that contains the given box transformed by `lhs'
dmnsn_aabb
dmnsn_transform_aabb(dmnsn_matrix M, dmnsn_aabb box)
{
  // Infinite/zero bounding box support
  if (isinf(box.min.X)) {
    return box;
  }

  // Taking the "absolute value" of the matrix saves some min/max calculations
  dmnsn_vector Mabs[3];
  for (unsigned int i = 0; i < 3; ++i) {
    for (unsigned int j = 0; j < 3; ++j) {
      Mabs[i].n[j] = fabs(M.n[j][i]);
    }
  }

  dmnsn_vector Mt = dmnsn_matrix_column(M, 3);
  dmnsn_aabb ret = { Mt, Mt };

  for (unsigned int i = 0; i < 3; ++i) {
    ret.min = dmnsn_vector_add(ret.min, dmnsn_vector_mul(box.min.n[i], Mabs[i]));
    ret.max = dmnsn_vector_add(ret.max, dmnsn_vector_mul(box.max.n[i], Mabs[i]));
  }

  return ret;
}
