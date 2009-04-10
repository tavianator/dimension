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

namespace dimension
{
  typedef dmnsn_scalar scalar;

  class vector
  {
  public:
    vector() { }
    vector(scalar x, scalar y, scalar z)
      : m_vector(dmnsn_vector_construct(x, y, z)) { }
    explicit vector(dmnsn_vector v) : m_vector(v) { }
    // vector(const vector& v);
    // ~vector();

    scalar x() const { return m_vector.x; }
    scalar y() const { return m_vector.y; }
    scalar z() const { return m_vector.z; }

    // vector& operator=(const vector& rhs);
    vector& operator+=(const vector& rhs)
      { m_vector = dmnsn_vector_add(m_vector, rhs.m_vector); return *this; }
    vector& operator-=(const vector& rhs)
      { m_vector = dmnsn_vector_sub(m_vector, rhs.m_vector); return *this; }
    vector& operator*=(scalar rhs)
      { m_vector = dmnsn_vector_mul(rhs, m_vector); return *this; }
    vector& operator/=(scalar rhs)
      { m_vector = dmnsn_vector_div(m_vector, rhs); return *this; }

    dmnsn_vector dmnsn() const { return m_vector; }

  private:
    dmnsn_vector m_vector;
  };

  class line
  {
  public:
    line() { }
    line(const vector& x0, const vector& n)
      { m_line.x0 = x0.dmnsn(); m_line.n = n.dmnsn(); }
    // line(const line& l);
    // ~line();

    vector x0() const { return vector(m_line.x0); }
    vector n()  const { return vector(m_line.n); }

    // line& operator=(const line& l);
    vector operator()(scalar t) { return vector(dmnsn_line_point(m_line, t)); }

    dmnsn_line dmnsn() const { return m_line; }

  private:
    dmnsn_line m_line;
  };

  // Vector operators

  inline vector
  operator+(const vector& lhs, const vector& rhs)
  {
    vector r = lhs;
    r += rhs;
    return r;
  }

  inline vector
  operator-(const vector& lhs, const vector& rhs)
  {
    vector r = lhs;
    r -= rhs;
    return r;
  }

  inline vector
  operator*(const vector& lhs, scalar rhs)
  {
    vector r = lhs;
    r *= rhs;
    return r;
  }

  inline vector
  operator*(scalar lhs, const vector& rhs)
  {
    vector r = rhs;
    r *= lhs;
    return r;
  }

  inline vector
  operator/(const vector& lhs, scalar rhs)
  {
    vector r = lhs;
    r /= rhs;
    return r;
  }

  inline scalar
  dot(const vector& lhs, const vector& rhs)
  {
    return dmnsn_vector_dot(lhs.dmnsn(), rhs.dmnsn());
  }

  inline vector
  cross(const vector& lhs, const vector& rhs)
  {
    return vector(dmnsn_vector_cross(lhs.dmnsn(), rhs.dmnsn()));
  }
}

#endif /* DIMENSIONXX_GEOMETRY_HPP */
