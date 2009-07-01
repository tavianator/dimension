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

#include "dimensionxx.hpp"

namespace Dimension
{
  // Pure virtual no-op destructor
  Object::~Object()
  { }

  Matrix
  Object::trans()
  {
    return Matrix(m_object->trans);
  }

  void
  Object::trans(const Matrix& trans)
  {
    m_object->trans = trans.dmnsn();
  }

  // Intersection list for the line l
  Array<double>
  Object::intersections(const Line& l)
  {
    return Array<double>(m_object->intersections_fn(m_object, l.dmnsn()));
  }

  // Whether the point `point' is inside the object
  bool
  Object::inside(const Vector& point)
  {
    return m_object->inside_fn(m_object, point.dmnsn());
  }

  // Return the wrapped object
  dmnsn_object*
  Object::dmnsn()
  {
    return m_object;
  }

  // Return a const version of the wrapped canvas
  const dmnsn_object*
  Object::dmnsn() const
  {
    return m_object;
  }

  // Protected default no-op constructor
  Object::Object()
  { }

  // Protected manual constructor
  Object::Object(dmnsn_object *object)
    : m_object(object)
  { }

  // Custom object callbacks
  namespace {
    dmnsn_array *
    intersections_fn(const dmnsn_object *object, dmnsn_line line)
    {
      Custom_Object* cobject = reinterpret_cast<Custom_Object*>(object->ptr);
      return cobject->intersections(Line(line)).release();
    }

    int
    inside_fn(const dmnsn_object *object, dmnsn_vector point)
    {
      Custom_Object* cobject = reinterpret_cast<Custom_Object*>(object->ptr);
      return cobject->inside(Vector(point));
    }
  }

  // Initialize a new object, using member functions as callbacks
  Custom_Object::Custom_Object()
    : Object(dmnsn_new_object())
  {
    m_object->ptr = this;
    m_object->intersections_fn = &intersections_fn;
    m_object->inside_fn = &inside_fn;
  }

  // Delete the object
  Custom_Object::~Custom_Object()
  {
    dmnsn_delete_object(m_object);
  }
}
