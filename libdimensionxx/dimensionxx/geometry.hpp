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

#ifndef DIMENSIONXX_GEOMETRY_HPP
#define DIMENSIONXX_GEOMETRY_HPP

#include <dimension.h>

namespace Dimension
{
  typedef dmnsn_scalar Scalar;

  class Vector
  {
  public:
    Vector() { }
    Vector(Scalar x, Scalar y, Scalar z)
      : m_vector(dmnsn_vector_construct(x, y, z)) { }
    explicit Vector(dmnsn_vector v) : m_vector(v) { }
    // Vector(const Vector& v);
    // ~Vector();

    Scalar x() const { return m_vector.x; }
    Scalar y() const { return m_vector.y; }
    Scalar z() const { return m_vector.z; }

    // Vector& operator=(const Vector& rhs);
    Vector& operator+=(const Vector& rhs)
      { m_vector = dmnsn_vector_add(m_vector, rhs.m_vector); return *this; }
    Vector& operator-=(const Vector& rhs)
      { m_vector = dmnsn_vector_sub(m_vector, rhs.m_vector); return *this; }
    Vector& operator*=(Scalar rhs)
      { m_vector = dmnsn_vector_mul(rhs, m_vector); return *this; }
    Vector& operator/=(Scalar rhs)
      { m_vector = dmnsn_vector_div(m_vector, rhs); return *this; }

    dmnsn_vector dmnsn() const { return m_vector; }

  private:
    dmnsn_vector m_vector;
  };

  class line
  {
  public:
    line() { }
    line(const Vector& x0, const Vector& n)
      { m_line.x0 = x0.dmnsn(); m_line.n = n.dmnsn(); }
    // line(const line& l);
    // ~line();

    Vector x0() const { return Vector(m_line.x0); }
    Vector n()  const { return Vector(m_line.n); }

    // line& operator=(const line& l);
    Vector operator()(Scalar t) { return Vector(dmnsn_line_point(m_line, t)); }

    dmnsn_line dmnsn() const { return m_line; }

  private:
    dmnsn_line m_line;
  };

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
  operator*(const Vector& lhs, Scalar rhs)
  {
    Vector r = lhs;
    r *= rhs;
    return r;
  }

  inline Vector
  operator*(Scalar lhs, const Vector& rhs)
  {
    Vector r = rhs;
    r *= lhs;
    return r;
  }

  inline Vector
  operator/(const Vector& lhs, Scalar rhs)
  {
    Vector r = lhs;
    r /= rhs;
    return r;
  }

  inline Scalar
  dot(const Vector& lhs, const Vector& rhs)
  {
    return dmnsn_vector_dot(lhs.dmnsn(), rhs.dmnsn());
  }

  inline Vector
  cross(const Vector& lhs, const Vector& rhs)
  {
    return Vector(dmnsn_vector_cross(lhs.dmnsn(), rhs.dmnsn()));
  }
}

#endif /* DIMENSIONXX_GEOMETRY_HPP */
