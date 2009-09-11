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

// Wrappers for geometric types (Vectors, Matricies, Lines (rays)).

#ifndef DIMENSIONXX_GEOMETRY_HPP
#define DIMENSIONXX_GEOMETRY_HPP

#include <dimension.h>

namespace Dimension
{
  class Vector;

  // Wrapper for dmnsn_matrix
  class Matrix
  {
  public:
    Matrix() { }
    Matrix(double a0, double a1, double a2, double a3,
           double b0, double b1, double b2, double b3,
           double c0, double c1, double c2, double c3,
           double d0, double d1, double d2, double d3)
      : m_matrix(dmnsn_matrix_construct(a0, a1, a2, a3,
                                        b0, b1, b2, b3,
                                        c0, c1, c2, c3,
                                        d0, d1, d2, d3)) { }
    explicit Matrix(dmnsn_matrix m) : m_matrix(m) { }
    // Matrix(const Matrix& m);
    // ~Matrix();

    // Element access
    double* operator[](unsigned int i) { return m_matrix.n[i]; }

    // Matrix arithmetic

    // Matrix& operator=(const Matrix& rhs);
    Matrix& operator*=(const Matrix& rhs)
      { m_matrix = dmnsn_matrix_mul(rhs.m_matrix, m_matrix); return *this; }

    // Get the wrapped matrix
    dmnsn_matrix dmnsn() const { return m_matrix; }

    // Special constructors
    static inline Matrix identity();
    static inline Matrix scale(const Vector& factor);
    static inline Matrix translation(const Vector& d);
    static inline Matrix rotation(const Vector& theta);

  private:
    dmnsn_matrix m_matrix;
  };

  // Wrapper for dmnsn_vector
  class Vector
  {
  public:
    Vector() { }
    Vector(double x, double y, double z)
      : m_vector(dmnsn_vector_construct(x, y, z)) { }
    explicit Vector(dmnsn_vector v) : m_vector(v) { }
    // Vector(const Vector& v);
    // ~Vector();

    // Get the x, y, and z components.
    double x() const { return m_vector.x; }
    double y() const { return m_vector.y; }
    double z() const { return m_vector.z; }

    // Vector arithmetic

    // Vector& operator=(const Vector& rhs);
    Vector& operator+=(const Vector& rhs)
      { m_vector = dmnsn_vector_add(m_vector, rhs.m_vector); return *this; }
    Vector& operator-=(const Vector& rhs)
      { m_vector = dmnsn_vector_sub(m_vector, rhs.m_vector); return *this; }
    Vector& operator*=(double rhs)
      { m_vector = dmnsn_vector_mul(rhs, m_vector); return *this; }
    Vector& operator*=(const Matrix& m)
      { m_vector = dmnsn_matrix_vector_mul(m.dmnsn(), m_vector); return *this; }
    Vector& operator/=(double rhs)
      { m_vector = dmnsn_vector_div(m_vector, rhs); return *this; }

    // Get the wrapped vector
    dmnsn_vector dmnsn() const { return m_vector; }

  private:
    dmnsn_vector m_vector;
  };

  // Wrapper for dmnsn_line
  class Line
  {
  public:
    Line() { }
    Line(const Vector& x0, const Vector& n)
      { m_line.x0 = x0.dmnsn(); m_line.n = n.dmnsn(); }
    explicit Line(dmnsn_line l) : m_line(l) { }
    // Line(const Line& l);
    // ~Line();

    Vector x0() const { return Vector(m_line.x0); }
    Vector n()  const { return Vector(m_line.n); }
    double t(const Vector& v) { return dmnsn_line_index(m_line, v.dmnsn()); }

    // Line& operator=(const Line& l);
    Line& operator*=(const Matrix& m)
      { m_line = dmnsn_matrix_line_mul(m.dmnsn(), m_line); return *this; }

    // Get the point `t' on the line (x0 + t*n)
    Vector operator()(double t) { return Vector(dmnsn_line_point(m_line, t)); }

    // Get the wrapped line
    dmnsn_line dmnsn() const { return m_line; }

  private:
    dmnsn_line m_line;
  };

  // Array_Element specializations

  template <>
  class Array_Element<Matrix>
    : public By_Value_Array_Element<Matrix, dmnsn_matrix>
  {
  public:
    typedef dmnsn_matrix C_Type;

    Array_Element() { }
    Array_Element(Matrix& matrix)
      : By_Value_Array_Element<Matrix, dmnsn_matrix>(matrix) { }
    Array_Element(C_Type c)
      : By_Value_Array_Element<Matrix, dmnsn_matrix>(c) { }
    // Array_Element(const Array_Element& ae);
    // ~Array_Element();

    // Array_Element& operator=(const Array_Element& ae);
  };

  template <>
  class Array_Element<Vector>
    : public By_Value_Array_Element<Vector, dmnsn_vector>
  {
  public:
    typedef dmnsn_vector C_Type;

    Array_Element() { }
    Array_Element(Vector& vector)
      : By_Value_Array_Element<Vector, dmnsn_vector>(vector) { }
    Array_Element(C_Type c)
      : By_Value_Array_Element<Vector, dmnsn_vector>(c) { }
    // Array_Element(const Array_Element& ae);
    // ~Array_Element();

    // Array_Element& operator=(const Array_Element& ae);
  };

  template <>
  class Array_Element<Line>
    : public By_Value_Array_Element<Line, dmnsn_line>
  {
  public:
    typedef dmnsn_line C_Type;

    Array_Element() { }
    Array_Element(Line& line)
      : By_Value_Array_Element<Line, dmnsn_line>(line) { }
    Array_Element(C_Type c)
      : By_Value_Array_Element<Line, dmnsn_line>(c) { }
    // Array_Element(const Array_Element& ae);
    // ~Array_Element();

    // Array_Element& operator=(const Array_Element& ae);
  };

  // Matrix operators

  inline Matrix
  operator*(const Matrix& lhs, const Matrix& rhs)
  {
    // This order is important!
    Matrix r = rhs;
    r *= lhs;
    return r;
  }

  inline Matrix
  inverse(const Matrix& M)
  {
    return Matrix(dmnsn_matrix_inverse(M.dmnsn()));
  }

  // Special Matrix constructors

  inline Matrix
  Matrix::identity()
  {
    return Matrix(dmnsn_identity_matrix());
  }

  inline Matrix
  Matrix::scale(const Vector& factor)
  {
    return Matrix(dmnsn_scale_matrix(factor.dmnsn()));
  }

  inline Matrix
  Matrix::translation(const Vector& d)
  {
    return Matrix(dmnsn_translation_matrix(d.dmnsn()));
  }

  inline Matrix
  Matrix::rotation(const Vector& theta)
  {
    return Matrix(dmnsn_rotation_matrix(theta.dmnsn()));
  }

  // Vector operators

  inline Vector
  operator+(const Vector& lhs, const Vector& rhs)
  {
    Vector r = lhs;
    r += rhs;
    return r;
  }

  inline Vector
  operator-(const Vector& lhs, const Vector& rhs)
  {
    Vector r = lhs;
    r -= rhs;
    return r;
  }

  inline Vector
  operator*(const Vector& lhs, double rhs)
  {
    Vector r = lhs;
    r *= rhs;
    return r;
  }

  inline Vector
  operator*(double lhs, const Vector& rhs)
  {
    Vector r = rhs;
    r *= lhs;
    return r;
  }

  inline Vector
  operator*(const Matrix& lhs, const Vector& rhs)
  {
    Vector r = rhs;
    r *= lhs;
    return r;
  }

  inline Vector
  operator/(const Vector& lhs, double rhs)
  {
    Vector r = lhs;
    r /= rhs;
    return r;
  }

  inline double
  norm(const Vector& v)
  {
    return dmnsn_vector_norm(v.dmnsn());
  }

  inline Vector
  normalize(const Vector& v)
  {
    return Vector(dmnsn_vector_normalize(v.dmnsn()));
  }

  // Dot product
  inline double
  dot(const Vector& lhs, const Vector& rhs)
  {
    return dmnsn_vector_dot(lhs.dmnsn(), rhs.dmnsn());
  }

  // Cross product
  inline Vector
  cross(const Vector& lhs, const Vector& rhs)
  {
    return Vector(dmnsn_vector_cross(lhs.dmnsn(), rhs.dmnsn()));
  }

  // Line transformation
  inline Line
  operator*(const Matrix& lhs, const Line& rhs)
  {
    Line r = rhs;
    r *= lhs;
    return r;
  }
}

#endif /* DIMENSIONXX_GEOMETRY_HPP */
