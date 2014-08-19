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
#define DMNSN_MATRIX_PRINTF(m)                          \
  (m).n[0][0], (m).n[0][1], (m).n[0][2], (m).n[0][3],   \
  (m).n[1][0], (m).n[1][1], (m).n[1][2], (m).n[1][3],   \
  (m).n[2][0], (m).n[2][1], (m).n[2][2], (m).n[2][3],   \
  0.0, 0.0, 0.0, 1.0

/** Construct a new transformation matrix. */
DMNSN_INLINE dmnsn_matrix
dmnsn_new_matrix(double a0, double a1, double a2, double a3,
                 double b0, double b1, double b2, double b3,
                 double c0, double c1, double c2, double c3)
{
  dmnsn_matrix m = { { { a0, a1, a2, a3 },
                       { b0, b1, b2, b3 },
                       { c0, c1, c2, c3 } } };
  return m;
}

/** Construct a new transformation matrix from column vectors. */
DMNSN_INLINE dmnsn_matrix
dmnsn_new_matrix4(dmnsn_vector a, dmnsn_vector b, dmnsn_vector c,
                  dmnsn_vector d)
{
  dmnsn_matrix m = { { { a.x, b.x, c.x, d.x },
                       { a.y, b.y, c.y, d.y },
                       { a.z, b.z, c.z, d.z } } };
  return m;
}

/** Extract column vectors from a matrix. */
DMNSN_INLINE dmnsn_vector
dmnsn_matrix_column(dmnsn_matrix M, unsigned int i)
{
  return dmnsn_new_vector(M.n[0][i], M.n[1][i], M.n[2][i]);
}

/** Return the identity matrix. */
dmnsn_matrix dmnsn_identity_matrix(void);

/**
 * A scale transformation.
 * @param[in] s  A vector with components representing the scaling factor in
 *               each axis.
 * @return The transformation matrix.
 */
dmnsn_matrix dmnsn_scale_matrix(dmnsn_vector s);
/**
 * A translation.
 * @param[in] d  The vector to translate by.
 * @return The transformation matrix.
 */
dmnsn_matrix dmnsn_translation_matrix(dmnsn_vector d);
/**
 * A left-handed rotation.
 * @param[in] theta  A vector representing an axis and angle.
 *                   @f$ axis = \vec{\theta}/|\vec{\theta}| @f$,
 *                   @f$ angle = |\vec{\theta}| @f$
 * @return The transformation matrix.
 */
dmnsn_matrix dmnsn_rotation_matrix(dmnsn_vector theta);

/**
 * An alignment matrix.
 * @param[in] from   The initial vector.
 * @param[in] to     The desired direction.
 * @param[in] axis1  The first axis about which to rotate.
 * @param[in] axis2  The second axis about which to rotate.
 * @return A transformation matrix that will rotate \p from to \p to.
 */
dmnsn_matrix dmnsn_alignment_matrix(dmnsn_vector from, dmnsn_vector to,
                                    dmnsn_vector axis1, dmnsn_vector axis2);

/** Invert a matrix. */
dmnsn_matrix dmnsn_matrix_inverse(dmnsn_matrix A);

/** Multiply two matricies. */
dmnsn_matrix dmnsn_matrix_mul(dmnsn_matrix lhs, dmnsn_matrix rhs);

/** Transform a point by a matrix. */
DMNSN_INLINE dmnsn_vector
dmnsn_transform_point(dmnsn_matrix T, dmnsn_vector v)
{
  /* 9 multiplications, 9 additions */
  dmnsn_vector r;
  r.x = T.n[0][0]*v.x + T.n[0][1]*v.y + T.n[0][2]*v.z + T.n[0][3];
  r.y = T.n[1][0]*v.x + T.n[1][1]*v.y + T.n[1][2]*v.z + T.n[1][3];
  r.z = T.n[2][0]*v.x + T.n[2][1]*v.y + T.n[2][2]*v.z + T.n[2][3];
  return r;
}

/** Transform a direction by a matrix. */
DMNSN_INLINE dmnsn_vector
dmnsn_transform_direction(dmnsn_matrix T, dmnsn_vector v)
{
  /* 9 multiplications, 6 additions */
  dmnsn_vector r;
  r.x = T.n[0][0]*v.x + T.n[0][1]*v.y + T.n[0][2]*v.z;
  r.y = T.n[1][0]*v.x + T.n[1][1]*v.y + T.n[1][2]*v.z;
  r.z = T.n[2][0]*v.x + T.n[2][1]*v.y + T.n[2][2]*v.z;
  return r;
}

/**
 * Transform a pseudovector by a matrix.
 * @param[in] Tinv  The inverse of the transformation matrix.
 * @param[in] v     The pseudovector to transform
 * @return The transformed pseudovector.
 */
DMNSN_INLINE dmnsn_vector
dmnsn_transform_normal(dmnsn_matrix Tinv, dmnsn_vector v)
{
  /* Multiply by the transpose of the inverse
     (9 multiplications, 6 additions) */
  dmnsn_vector r;
  r.x = Tinv.n[0][0]*v.x + Tinv.n[1][0]*v.y + Tinv.n[2][0]*v.z;
  r.y = Tinv.n[0][1]*v.x + Tinv.n[1][1]*v.y + Tinv.n[2][1]*v.z;
  r.z = Tinv.n[0][2]*v.x + Tinv.n[1][2]*v.y + Tinv.n[2][2]*v.z;
  return r;
}

/**
 * Transform a ray by a matrix.
 * \f$ n' = T(l.\vec{x_0} + l.\vec{n}) - T(l.\vec{x_0}) \f$,
 * \f$ \vec{x_0}' = T(l.\vec{x_0}) \f$
 */
DMNSN_INLINE dmnsn_ray
dmnsn_transform_ray(dmnsn_matrix T, dmnsn_ray l)
{
  /* 18 multiplications, 15 additions */
  dmnsn_ray ret;
  ret.x0 = dmnsn_transform_point(T, l.x0);
  ret.n  = dmnsn_transform_direction(T, l.n);
  return ret;
}

/** Transform a bounding box by a matrix. */
dmnsn_aabb dmnsn_transform_aabb(dmnsn_matrix T, dmnsn_aabb box);

/** Return whether a matrix contains any NaN components. */
DMNSN_INLINE bool
dmnsn_matrix_isnan(dmnsn_matrix m)
{
  size_t i, j;
  for (i = 0; i < 3; ++i) {
    for (j = 0; j < 4; ++j) {
      if (dmnsn_isnan(m.n[i][j])) {
        return true;
      }
    }
  }
  return false;
}
