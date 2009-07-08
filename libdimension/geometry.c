/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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
  /* Two trig calls, 25 multiplications, 13 additions */
  dmnsn_vector axis;
  double angle, s, t, x, y, z;

  angle = dmnsn_vector_norm(theta);
  if (angle == 0.0) {
    return dmnsn_identity_matrix();
  }
  axis = dmnsn_vector_normalize(theta);

  /* Shorthand to make dmnsn_matrix_construct() call legible */

  s = sin(angle);
  t = 1.0 - cos(angle);

  x = axis.x;
  y = axis.y;
  z = axis.z;

  return dmnsn_matrix_construct(
    1.0 + t*(x*x - 1.0), -z*s + t*x*y,        y*s + t*x*z,         0.0,
    z*s + t*x*y,         1.0 + t*(y*y - 1.0), -x*s + t*y*z,        0.0,
    -y*s + t*x*z,        x*s + t*y*z,         1.0 + t*(z*z - 1.0), 0.0,
    0.0,                 0.0,                 0.0,                 1.0
  );
}

/* Matrix inversion helper functions */

typedef struct { double n[2][2]; } dmnsn_matrix2;

static dmnsn_matrix2 dmnsn_matrix2_construct(double a1, double a2,
                                             double b1, double b2);
static dmnsn_matrix2 dmnsn_matrix2_inverse(dmnsn_matrix2 A);
static dmnsn_matrix2 dmnsn_matrix2_negate(dmnsn_matrix2 A);
static dmnsn_matrix2 dmnsn_matrix2_sub(dmnsn_matrix2 lhs, dmnsn_matrix2 rhs);
static dmnsn_matrix2 dmnsn_matrix2_mul(dmnsn_matrix2 lhs, dmnsn_matrix2 rhs);

static dmnsn_matrix dmnsn_matrix_inverse_generic(dmnsn_matrix A);
static double dmnsn_matrix_cofactor(dmnsn_matrix A,
                                    unsigned int row, unsigned int col);

/* Invert a matrix, by partitioning */
dmnsn_matrix
dmnsn_matrix_inverse(dmnsn_matrix A)
{
  /*
   * Use partitioning to invert a matrix:
   *
   *     ( P Q ) -1
   *     ( R S )
   *
   *   = ( PP QQ )
   *     ( RR SS ),
   *
   * with PP = inv(P) - inv(P)*Q*RR,
   *      QQ = -inv(P)*Q*SS,
   *      RR = -SS*R*inv(P), and
   *      SS = inv(S - R*inv(P)*Q).
   */

  /* The algorithm uses 2 inversions, 6 multiplications, and 2 subtractions,
     giving 52 multiplications, 34 additions, and 8 divisions. */

  dmnsn_matrix2 P, Q, R, S, Pi, RPi, PiQ, RPiQ, PP, QQ, RR, SS;
  double Pdet = A.n[0][0]*A.n[1][1] - A.n[0][1]*A.n[1][0];

  if (Pdet == 0.0) {
    /* If we can't invert P, try a more generic algorithm; this is very
       unlikely, but not impossible, eg.
         ( 1 1 0 0 )
         ( 1 1 1 0 )
         ( 0 1 1 0 )
         ( 0 0 0 1 )
     */
    return dmnsn_matrix_inverse_generic(A);
  }

  /* Partition the matrix */
  P = dmnsn_matrix2_construct(A.n[0][0], A.n[0][1],
                              A.n[1][0], A.n[1][1]);
  Q = dmnsn_matrix2_construct(A.n[0][2], A.n[0][3],
                              A.n[1][2], A.n[1][3]);
  R = dmnsn_matrix2_construct(A.n[2][0], A.n[2][1],
                              A.n[3][0], A.n[3][1]);
  S = dmnsn_matrix2_construct(A.n[2][2], A.n[2][3],
                              A.n[3][2], A.n[3][3]);

  /* Do this inversion ourselves, since we already have the determinant */
  Pi = dmnsn_matrix2_construct( P.n[1][1]/Pdet, -P.n[0][1]/Pdet,
                               -P.n[1][0]/Pdet,  P.n[0][0]/Pdet);

  /* Calculate R*inv(P), inv(P)*Q, and R*inv(P)*Q */
  RPi  = dmnsn_matrix2_mul(R, Pi);
  PiQ  = dmnsn_matrix2_mul(Pi, Q);
  RPiQ = dmnsn_matrix2_mul(R, PiQ);

  /* Calculate the partitioned inverse */
  SS = dmnsn_matrix2_inverse(dmnsn_matrix2_sub(S, RPiQ));
  RR = dmnsn_matrix2_negate(dmnsn_matrix2_mul(SS, RPi));
  QQ = dmnsn_matrix2_negate(dmnsn_matrix2_mul(PiQ, SS));
  PP = dmnsn_matrix2_sub(Pi, dmnsn_matrix2_mul(PiQ, RR));

  /* Reconstruct the matrix */
  return dmnsn_matrix_construct(PP.n[0][0], PP.n[0][1], QQ.n[0][0], QQ.n[0][1],
                                PP.n[1][0], PP.n[1][1], QQ.n[1][0], QQ.n[1][1],
                                RR.n[0][0], RR.n[0][1], SS.n[0][0], SS.n[0][1],
                                RR.n[1][0], RR.n[1][1], SS.n[1][0], SS.n[1][1]);
}

/* For nice shorthand */
static dmnsn_matrix2
dmnsn_matrix2_construct(double a1, double a2, double b1, double b2)
{
  dmnsn_matrix2 m = { { { a1, a2 },
                        { b1, b2 } } };
  return m;
}

/* Invert a 2x2 matrix */
static dmnsn_matrix2
dmnsn_matrix2_inverse(dmnsn_matrix2 A)
{
  /* 4 divisions, 2 multiplications, 1 addition */
  double det = A.n[0][0]*A.n[1][1] - A.n[0][1]*A.n[1][0];
  return dmnsn_matrix2_construct( A.n[1][1]/det, -A.n[0][1]/det,
                                 -A.n[1][0]/det,  A.n[0][0]/det);
}

/* Also basically a shorthand */
static dmnsn_matrix2
dmnsn_matrix2_negate(dmnsn_matrix2 A)
{
  return dmnsn_matrix2_construct(-A.n[0][0], -A.n[0][1],
                                 -A.n[1][0], -A.n[1][1]);
}

/* 2x2 matrix subtraction */
static dmnsn_matrix2
dmnsn_matrix2_sub(dmnsn_matrix2 lhs, dmnsn_matrix2 rhs)
{
  /* 4 additions */
  return dmnsn_matrix2_construct(
    lhs.n[0][0] - rhs.n[0][0], lhs.n[0][1] - rhs.n[0][1],
    lhs.n[1][0] - rhs.n[1][0], lhs.n[1][1] - rhs.n[1][1]
  );
}

/* 2x2 matrix multiplication */
static dmnsn_matrix2
dmnsn_matrix2_mul(dmnsn_matrix2 lhs, dmnsn_matrix2 rhs)
{
  /* 8 multiplications, 4 additions */
  return dmnsn_matrix2_construct(
    lhs.n[0][0]*rhs.n[0][0] + lhs.n[0][1]*rhs.n[1][0],
      lhs.n[0][0]*rhs.n[0][1] + lhs.n[0][1]*rhs.n[1][1],
    lhs.n[1][0]*rhs.n[0][0] + lhs.n[1][1]*rhs.n[1][0],
      lhs.n[1][0]*rhs.n[0][1] + lhs.n[1][1]*rhs.n[1][1]
  );
}

/* Invert a matrix, if partitioning failed (|P| == 0) */
static dmnsn_matrix
dmnsn_matrix_inverse_generic(dmnsn_matrix A)
{
  /*
   * Simply form the matrix's adjugate and divide each element by the
   * determinant as we go.  The routine itself has 4 additions and 16 divisions
   * plus 16 cofactor calculations, giving 144 multiplications, 84 additions,
   * and 16 divisions.
   */
  dmnsn_matrix inv;
  double det = 0.0, C;
  unsigned int i, j;

  /* Perform a Laplace expansion along the first row to give us the adjugate's
     first column and the determinant */
  for (j = 0; j < 4; ++j) {
    C = dmnsn_matrix_cofactor(A, 0, j);
    det += A.n[0][j]*C;
    inv.n[j][0] = C;
  }

  /* Divide the first column by the determinant */
  for (j = 0; j < 4; ++j) {
    inv.n[j][0] /= det;
  }

  /* Find columns 2 through 4 */
  for (i = 1; i < 4; ++i) {
    for (j = 0; j < 4; ++j) {
      inv.n[j][i] = dmnsn_matrix_cofactor(A, i, j)/det;
    }
  }

  return inv;
}

/* Gives the cofactor at row, col; the determinant of the matrix formed from A
   by ignoring row `row' and column `col', times (-1)**(row + col) */
static double
dmnsn_matrix_cofactor(dmnsn_matrix A, unsigned int row, unsigned int col)
{
  /* 9 multiplications, 5 additions */
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
  /* 64 multiplications, 48 additions */
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
  /* 12 multiplications, 3 divisions, 12 additions */
  dmnsn_vector r;
  double w;

  r.x = lhs.n[0][0]*rhs.x + lhs.n[0][1]*rhs.y + lhs.n[0][2]*rhs.z + lhs.n[0][3];
  r.y = lhs.n[1][0]*rhs.x + lhs.n[1][1]*rhs.y + lhs.n[1][2]*rhs.z + lhs.n[1][3];
  r.z = lhs.n[2][0]*rhs.x + lhs.n[2][1]*rhs.y + lhs.n[2][2]*rhs.z + lhs.n[2][3];
  w   = lhs.n[3][0]*rhs.x + lhs.n[3][1]*rhs.y + lhs.n[3][2]*rhs.z + lhs.n[3][3];

  return dmnsn_vector_div(r, w);
}

/* Solve for the t value such that x0 + t*n = x */
double
dmnsn_line_index(dmnsn_line l, dmnsn_vector x)
{
  /* nz + 1 divisions, nz additions */
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
