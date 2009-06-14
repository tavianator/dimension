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

/*
 * Core geometric types like scalars, vectors, and rays.
 */

#ifndef DIMENSION_GEOMETRY_H
#define DIMENSION_GEOMETRY_H

/* Vector and matrix types. */

typedef struct { double x, y, z; } dmnsn_vector;

typedef struct {
  double n00, n01, n02, n03,
         n10, n11, n12, n13,
         n20, n21, n22, n23,
         n30, n31, n32, n33;
} dmnsn_matrix;

/* A line, or ray. */
typedef struct {
  dmnsn_vector x0; /* A point on the line */
  dmnsn_vector n;  /* A normal vector; the direction of the line */
} dmnsn_line;

/* Shorthand for vector/matrix construction */

dmnsn_vector dmnsn_vector_construct(double x, double y, double z);

dmnsn_matrix dmnsn_matrix_construct(double a0, double a1, double a2, double a3,
                                    double b0, double b1, double b2, double b3,
                                    double c0, double c1, double c2, double c3,
                                    double d0, double d1, double d2, double d3);

dmnsn_matrix dmnsn_identity_matrix();
dmnsn_matrix dmnsn_scale_matrix(dmnsn_vector s);
dmnsn_matrix dmnsn_translation_matrix(dmnsn_vector d);
/* Left-handed rotation; theta/|theta| = axis, |theta| = angle */
dmnsn_matrix dmnsn_rotation_matrix(dmnsn_vector theta);

/* Vector and matrix arithmetic */

dmnsn_vector dmnsn_vector_add(dmnsn_vector lhs, dmnsn_vector rhs);
dmnsn_vector dmnsn_vector_sub(dmnsn_vector lhs, dmnsn_vector rhs);
dmnsn_vector dmnsn_vector_mul(double lhs, dmnsn_vector rhs);
dmnsn_vector dmnsn_vector_div(dmnsn_vector lhs, double rhs);

double       dmnsn_vector_dot(dmnsn_vector lhs, dmnsn_vector rhs);
dmnsn_vector dmnsn_vector_cross(dmnsn_vector lhs, dmnsn_vector rhs);

double       dmnsn_vector_norm(dmnsn_vector n);
dmnsn_vector dmnsn_vector_normalize(dmnsn_vector n);

dmnsn_matrix dmnsn_matrix_mul(dmnsn_matrix lhs, dmnsn_matrix rhs);
dmnsn_vector dmnsn_matrix_vector_mul(dmnsn_matrix lhs, dmnsn_vector rhs);
dmnsn_line   dmnsn_matrix_line_mul(dmnsn_matrix lhs, dmnsn_line rhs);

/* A point on a line, defined by x0 + t*n */
dmnsn_vector dmnsn_line_point(dmnsn_line l, double t);

#endif /* DIMENSION_GEOMETRY_H */
