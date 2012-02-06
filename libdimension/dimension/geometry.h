/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Core geometric types like vectors, matricies, and rays.
 */

#include <math.h>
#include <stdbool.h>

/** A vector in 3 dimensions. */
typedef struct dmnsn_vector {
  double x; /**< The x component. */
  double y; /**< The y component. */
  double z; /**< The z component. */
} dmnsn_vector;

/** A standard format string for vectors. */
#define DMNSN_VECTOR_FORMAT "<%g, %g, %g>"
/** The appropriate arguements to printf() a vector. */
#define DMNSN_VECTOR_PRINTF(v) (v).x, (v).y, (v).z

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

/** A line, or ray. */
typedef struct dmnsn_line {
  dmnsn_vector x0; /**< A point on the line. */
  dmnsn_vector n;  /**< A normal vector; the direction of the line. */
} dmnsn_line;

/** A standard format string for lines. */
#define DMNSN_LINE_FORMAT "(<%g, %g, %g> + t*<%g, %g, %g>)"
/** The appropriate arguements to printf() a line. */
#define DMNSN_LINE_PRINTF(l)                                    \
  DMNSN_VECTOR_PRINTF((l).x0), DMNSN_VECTOR_PRINTF((l).n)

/** An axis-aligned bounding box (AABB). */
typedef struct dmnsn_bounding_box {
  dmnsn_vector min; /**< The coordinate-wise minimum extent of the box. */
  dmnsn_vector max; /**< The coordinate-wise maximum extent of the box. */
} dmnsn_bounding_box;

/** A standard format string for bounding boxes. */
#define DMNSN_BOUNDING_BOX_FORMAT "(<%g, %g, %g> ==> <%g, %g, %g>)"
/** The appropriate arguements to printf() a bounding box. */
#define DMNSN_BOUNDING_BOX_PRINTF(box)                                  \
  DMNSN_VECTOR_PRINTF((box).min), DMNSN_VECTOR_PRINTF((box).max)

/* Constants */

/** The smallest value considered non-zero by some numerical algorithms. */
#define dmnsn_epsilon 1.0e-10

/** The zero vector. */
static const dmnsn_vector dmnsn_zero = { 0.0, 0.0, 0.0 };
/** The x vector. */
static const dmnsn_vector dmnsn_x = { 1.0, 0.0, 0.0 };
/** The y vector. */
static const dmnsn_vector dmnsn_y = { 0.0, 1.0, 0.0 };
/** The z vector. */
static const dmnsn_vector dmnsn_z = { 0.0, 0.0, 1.0 };

/* Scalar functions */

/** Find the minimum of two scalars. */
DMNSN_INLINE double
dmnsn_min(double a, double b)
{
  return a < b ? a : b;
}

/** Find the maximum of two scalars. */
DMNSN_INLINE double
dmnsn_max(double a, double b)
{
  return a > b ? a : b;
}

/** Convert degrees to radians. */
DMNSN_INLINE double
dmnsn_radians(double degrees)
{
  return degrees*atan(1.0)/45.0;
}

/** Convert radians to degrees. */
DMNSN_INLINE double
dmnsn_degrees(double radians)
{
  return radians*45.0/atan(1.0);
}

/** Return the sign of a scalar. */
DMNSN_INLINE int
dmnsn_sign(double n)
{
  if (n > 0.0) {
    return 1;
  } else if (n < 0.0) {
    return -1;
  } else {
    return 0;
  }
}

/* Shorthand for vector/matrix construction */

/** Construct a new vector. */
DMNSN_INLINE dmnsn_vector
dmnsn_new_vector(double x, double y, double z)
{
  dmnsn_vector v = { x, y, z };
  return v;
}

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

/**
 * Construct a new line.
 * @param[in] x0  A point on the line.
 * @param[in] n   The direction of the line.
 * @return The new line.
 */
DMNSN_INLINE dmnsn_line
dmnsn_new_line(dmnsn_vector x0, dmnsn_vector n)
{
  dmnsn_line l = { x0, n };
  return l;
}

/** Return the bounding box which contains nothing. */
DMNSN_INLINE dmnsn_bounding_box
dmnsn_zero_bounding_box(void)
{
  dmnsn_bounding_box box = {
    {  INFINITY,  INFINITY,  INFINITY },
    { -INFINITY, -INFINITY, -INFINITY }
  };
  return box;
}

/** Return the bounding box which contains everything. */
DMNSN_INLINE dmnsn_bounding_box
dmnsn_infinite_bounding_box(void)
{
  dmnsn_bounding_box box = {
    { -INFINITY, -INFINITY, -INFINITY },
    {  INFINITY,  INFINITY,  INFINITY }
  };
  return box;
}

/* Vector and matrix arithmetic */

/** Negate a vector. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_negate(dmnsn_vector rhs)
{
  /* 3 negations */
  dmnsn_vector v = { -rhs.x, -rhs.y, -rhs.z };
  return v;
}

/** Add two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_add(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 3 additions */
  dmnsn_vector v = { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
  return v;
}

/** Subtract two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_sub(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 3 additions */
  dmnsn_vector v = { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
  return v;
}

/** Multiply a vector by a scalar. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_mul(double lhs, dmnsn_vector rhs)
{
  /* 3 multiplications */
  dmnsn_vector v = { lhs*rhs.x, lhs*rhs.y, lhs*rhs.z };
  return v;
}

/** Divide a vector by a scalar. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_div(dmnsn_vector lhs, double rhs)
{
  /* 3 divisions */
  dmnsn_vector v = { lhs.x/rhs, lhs.y/rhs, lhs.z/rhs };
  return v;
}

/** Return the dot product of two vectors. */
DMNSN_INLINE double
dmnsn_vector_dot(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 3 multiplications, 2 additions */
  return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

/** Return the cross product of two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_cross(dmnsn_vector lhs, dmnsn_vector rhs)
{
  /* 6 multiplications, 3 additions */
  dmnsn_vector v = { lhs.y*rhs.z - lhs.z*rhs.y,
                     lhs.z*rhs.x - lhs.x*rhs.z,
                     lhs.x*rhs.y - lhs.y*rhs.x };
  return v;
}

/** Return the projection of \p u onto \p d. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_proj(dmnsn_vector u, dmnsn_vector d)
{
  /* 1 division, 9 multiplications, 4 additions */
  return dmnsn_vector_mul(dmnsn_vector_dot(u, d)/dmnsn_vector_dot(d, d), d);
}

/** Return the magnitude of a vector. */
DMNSN_INLINE double
dmnsn_vector_norm(dmnsn_vector n)
{
  /* 1 sqrt, 3 multiplications, 2 additions */
  return sqrt(dmnsn_vector_dot(n, n));
}

/** Return the direction of a vector. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_normalized(dmnsn_vector n)
{
  /* 1 sqrt, 3 divisions, 3 multiplications, 2 additions */
  return dmnsn_vector_div(n, dmnsn_vector_norm(n));
}

/** Return the component-wise minimum of two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_min(dmnsn_vector a, dmnsn_vector b)
{
  return dmnsn_new_vector(
    dmnsn_min(a.x, b.x),
    dmnsn_min(a.y, b.y),
    dmnsn_min(a.z, b.z)
  );
}

/** Return the component-wise maximum of two vectors. */
DMNSN_INLINE dmnsn_vector
dmnsn_vector_max(dmnsn_vector a, dmnsn_vector b)
{
  return dmnsn_new_vector(
    dmnsn_max(a.x, b.x),
    dmnsn_max(a.y, b.y),
    dmnsn_max(a.z, b.z)
  );
}

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
  /* 9 multiplications, 9 additions */
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

/** Transform a bounding box by a matrix. */
dmnsn_bounding_box dmnsn_transform_bounding_box(dmnsn_matrix T,
                                                dmnsn_bounding_box box);

/**
 * Transform a line by a matrix.
 * \f$ n' = T(l.\vec{x_0} + l.\vec{n}) - T(l.\vec{x_0}) \f$,
 * \f$ \vec{x_0}' = T(l.\vec{x_0}) \f$
 */
DMNSN_INLINE dmnsn_line
dmnsn_transform_line(dmnsn_matrix T, dmnsn_line l)
{
  /* 18 multiplications, 24 additions */
  dmnsn_line ret;
  ret.x0 = dmnsn_transform_point(T, l.x0);
  ret.n  = dmnsn_transform_direction(T, l.n);
  return ret;
}

/**
 * Return the point at \p t on a line.
 * The point is defined by \f$ l.\vec{x_0} + t \cdot l.\vec{n} \f$
 */
DMNSN_INLINE dmnsn_vector
dmnsn_line_point(dmnsn_line l, double t)
{
  return dmnsn_vector_add(l.x0, dmnsn_vector_mul(t, l.n));
}

/** Add epsilon*l.n to l.x0, to avoid self-intersections. */
DMNSN_INLINE dmnsn_line
dmnsn_line_add_epsilon(dmnsn_line l)
{
  return dmnsn_new_line(
    dmnsn_vector_add(
      l.x0,
      dmnsn_vector_mul(1.0e3*dmnsn_epsilon, l.n)
    ),
    l.n
  );
}

/** Return whether \p p is within the axis-aligned bounding box. */
DMNSN_INLINE bool
dmnsn_bounding_box_contains(dmnsn_bounding_box box, dmnsn_vector p)
{
  return (p.x >= box.min.x && p.y >= box.min.y && p.z >= box.min.z)
      && (p.x <= box.max.x && p.y <= box.max.y && p.z <= box.max.z);
}

/** Return whether a bounding box is infinite. */
DMNSN_INLINE bool
dmnsn_bounding_box_is_infinite(dmnsn_bounding_box box)
{
  return box.min.x == -INFINITY;
}

/** Return whether a vector contains any NaN components. */
DMNSN_INLINE bool
dmnsn_vector_isnan(dmnsn_vector v)
{
  return isnan(v.x) || isnan(v.y) || isnan(v.z);
}

/** Return whether a matrix contains any NaN components. */
DMNSN_INLINE bool
dmnsn_matrix_isnan(dmnsn_matrix m)
{
  size_t i, j;
  for (i = 0; i < 3; ++i) {
    for (j = 0; j < 4; ++j) {
      if (isnan(m.n[i][j])) {
        return true;
      }
    }
  }
  return false;
}

/** Return whether a line contains any NaN entries. */
DMNSN_INLINE bool
dmnsn_line_isnan(dmnsn_line l)
{
  return dmnsn_vector_isnan(l.x0) || dmnsn_vector_isnan(l.n);
}

/** Return whether a bounding box has any NaN components. */
DMNSN_INLINE bool
dmnsn_bounding_box_isnan(dmnsn_bounding_box box)
{
  return dmnsn_vector_isnan(box.min) || dmnsn_vector_isnan(box.max);
}
