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
 * Affine transformation matrices.
 */

#ifndef DMNSN_MATH_H
#error "Please include <dimension/math.h> instead of this header directly."
#endif

/** A 4x4 affine transformation matrix, with implied [0 0 0 1] bottom row. */
typedef struct dmnsn_matrix {
  double n[3][4]; /**< The matrix elements in row-major order. */
} dmnsn_matrix;

/** A standard format string for matricies. */
#define DMNSN_MATRIX_FORMAT                     \
  "[%g\t%g\t%g\t%g]\n"                          \
  "[%g\t%g\t%g\t%g]\n"                          \
  "[%g\t%g\t%g\t%g]\n"                          \
  "[%g\t%g\t%g\t%g]"
/** The appropriate arguements to printf() a matrix. */
#define DMNSN_MATRIX_PRINTF(M)                              \
  (M).n[0][0], (M).n[0][1], (M).n[0][2], (M).n[0][3],   \
  (M).n[1][0], (M).n[1][1], (M).n[1][2], (M).n[1][3],   \
  (M).n[2][0], (M).n[2][1], (M).n[2][2], (M).n[2][3],   \
  0.0, 0.0, 0.0, 1.0

/** Create a transformation matrix. */
DMNSN_INLINE dmnsn_matrix
dmnsn_new_matrix(double a0, double b0, double c0, double d0,
                 double a1, double b1, double c1, double d1,
                 double a2, double b2, double c2, double d2)
{
  dmnsn_matrix M = {
    {
      { a0, b0, c0, d0 },
      { a1, b1, c1, d1 },
      { a2, b2, c2, d2 }
    }
  };
  return M;
}

/** Create a transformation matrix from column vectors. */
DMNSN_INLINE dmnsn_matrix
dmnsn_new_matrix4(dmnsn_vector a, dmnsn_vector b, dmnsn_vector c, dmnsn_vector d)
{
  dmnsn_matrix M;

  unsigned int i;
  for (i = 0; i < 3; ++i) {
    M.n[i][0] = a.n[i];
  }
  for (i = 0; i < 3; ++i) {
    M.n[i][1] = b.n[i];
  }
  for (i = 0; i < 3; ++i) {
    M.n[i][2] = c.n[i];
  }
  for (i = 0; i < 3; ++i) {
    M.n[i][3] = d.n[i];
  }

  return M;
}

/** Extract column vectors from a matrix. */
DMNSN_INLINE dmnsn_vector
dmnsn_matrix_column(dmnsn_matrix M, unsigned int i)
{
  dmnsn_vector v;
  unsigned int j;
  for (j = 0; j < 3; ++j) {
    v.n[j] = M.n[j][i];
  }
  return v;
}

/** Return the identity matrix. */
DMNSN_INLINE dmnsn_matrix
dmnsn_identity_matrix(void)
{
  return dmnsn_new_matrix(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0
  );
}

/**
 * Return a scale transformation.
 * @param[in]  s  A vector with components representing the scaling factor in
 *                each axis.
 */
DMNSN_INLINE dmnsn_matrix
dmnsn_scale_matrix(dmnsn_vector s)
{
  return dmnsn_new_matrix(
    s.n[0], 0.0,    0.0,    0.0,
    0.0,    s.n[1], 0.0,    0.0,
    0.0,    0.0,    s.n[2], 0.0
  );
}

/**
 * Set \p M to a translation matrix.
 * @param[in]  d  The vector to translate by.
 */
DMNSN_INLINE dmnsn_matrix
dmnsn_translation_matrix(dmnsn_vector d)
{
  return dmnsn_new_matrix(
    1.0, 0.0, 0.0, d.n[0],
    0.0, 1.0, 0.0, d.n[1],
    0.0, 0.0, 1.0, d.n[2]
  );
}

/**
 * Return a rotation matrix.
 * @param[in]  theta  A vector representing an axis and angle.
 *                    @f$ axis = \vec{\theta}/|\vec{\theta}| @f$,
 *                    @f$ angle = |\vec{\theta}| @f$
 */
dmnsn_matrix dmnsn_rotation_matrix(dmnsn_vector theta);

/**
 * Return an alignment matrix.
 * @param[in] from   The initial vector.
 * @param[in] to     The desired direction.
 * @param[in] axis1  The first axis about which to rotate.
 * @param[in] axis2  The second axis about which to rotate.
 */
dmnsn_matrix dmnsn_alignment_matrix(dmnsn_vector from, dmnsn_vector to, dmnsn_vector axis1, dmnsn_vector axis2);

/** Invert a matrix. */
dmnsn_matrix dmnsn_matrix_inverse(dmnsn_matrix M);

/** Multiply two matricies. */
DMNSN_INLINE dmnsn_matrix dmnsn_matrix_mul(dmnsn_matrix lhs, dmnsn_matrix rhs)
{
  dmnsn_matrix M;

  unsigned int i, j, k;
  for (i = 0; i < 3; ++i) {
    for (j = 0; j < 3; ++j) {
      M.n[i][j] = 0.0;
      for (k = 0; k < 3; ++k) {
        M.n[i][j] += lhs.n[i][k]*rhs.n[k][j];
      }
    }

    M.n[i][3] = lhs.n[i][3];
    for (k = 0; k < 3; ++k) {
      M.n[i][3] += lhs.n[i][k]*rhs.n[k][3];
    }
  }

  return M;
}

/** Transform a point by a matrix. */
DMNSN_INLINE dmnsn_vector
dmnsn_transform_point(dmnsn_matrix M, dmnsn_vector v)
{
  dmnsn_vector r;
  unsigned int i, j;
  for (i = 0; i < 3; ++i) {
    r.n[i] = M.n[i][3];
    for (j = 0; j < 3; ++j) {
      r.n[i] += M.n[i][j]*v.n[j];
    }
  }
  return r;
}

/** Transform a direction by a matrix. */
DMNSN_INLINE dmnsn_vector
dmnsn_transform_direction(dmnsn_matrix M, dmnsn_vector v)
{
  dmnsn_vector r;
  unsigned int i, j;
  for (i = 0; i < 3; ++i) {
    r.n[i] = 0.0;
    for (j = 0; j < 3; ++j) {
      r.n[i] += M.n[i][j]*v.n[j];
    }
  }
  return r;
}

/**
 * Transform a pseudovector by a matrix.
 * @param[in] Minv  The inverse of the transformation matrix.
 * @param[in] v     The pseudovector to transform.
 * @return The transformed pseudovector.
 */
DMNSN_INLINE dmnsn_vector
dmnsn_transform_normal(dmnsn_matrix Minv, dmnsn_vector v)
{
  /* Multiply by the transpose of the inverse */
  dmnsn_vector r;
  unsigned int i, j;
  for (i = 0; i < 3; ++i) {
    r.n[i] = 0.0;
    for (j = 0; j < 3; ++j) {
      r.n[i] += Minv.n[j][i]*v.n[j];
    }
  }
  return r;
}

/** Transform a ray by a matrix. */
DMNSN_INLINE dmnsn_ray
dmnsn_transform_ray(dmnsn_matrix M, dmnsn_ray l)
{
  dmnsn_ray ret;
  ret.x0 = dmnsn_transform_point(M, l.x0);
  ret.n = dmnsn_transform_direction(M, l.n);
  return ret;
}

/** Transform a bounding box by a matrix. */
dmnsn_aabb dmnsn_transform_aabb(dmnsn_matrix M, dmnsn_aabb box);

/** Return whether a matrix contains any NaN components. */
DMNSN_INLINE bool
dmnsn_matrix_isnan(dmnsn_matrix M)
{
  unsigned int i, j;
  for (i = 0; i < 3; ++i) {
    for (j = 0; j < 4; ++j) {
      if (dmnsn_isnan(M.n[i][j])) {
        return true;
      }
    }
  }
  return false;
}
