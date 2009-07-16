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

#include "dimensionxx.hpp"

namespace Dimension
{
  // Construct an intersection object
  Intersection::Intersection(const Line& ray, double t, const Texture& texture)
    : m_intersection(new dmnsn_intersection*(dmnsn_new_intersection())),
      m_ray(ray), m_texture(texture)
  {
    dmnsn()->t = t;
  }

  // Wrap an existing intersection - don't release() one of these
  Intersection::Intersection(dmnsn_intersection *intersection)
    : m_intersection(new dmnsn_intersection*(intersection)),
      m_ray(intersection->ray),
      m_texture(const_cast<dmnsn_texture*>(intersection->texture))
  { }

  // Delete an intersection
  Intersection::~Intersection() {
    if (m_intersection && m_intersection.unique()) {
      dmnsn_delete_intersection(dmnsn());
    }
  }

  dmnsn_intersection*
  Intersection::dmnsn()
  {
    if (!m_intersection) {
      throw Dimension_Error("Attempt to access released intersection.");
    }
    return *m_intersection;
  }

  const dmnsn_intersection*
  Intersection::dmnsn() const
  {
    if (!m_intersection) {
      throw Dimension_Error("Attempt to access released intersection.");
    }
    return *m_intersection;
  }

  dmnsn_intersection*
  Intersection::release()
  {
    if (!m_intersection) {
      throw Dimension_Error("Attempt to release previously released"
                            " intersection.");
    }

    if (!m_intersection.unique()) {
      throw Dimension_Error("Attempt to release non-unique intersection.");
    }

    dmnsn_intersection* intersection = dmnsn();
    m_intersection.reset();
    return intersection;
  }

  // Manual constructor
  Object::Object(dmnsn_object* object)
    : m_object(new dmnsn_object*(object))
  { }

  // Virtual Object destructor
  Object::~Object()
  {
    if (unique()) {
      dmnsn_delete_object(dmnsn());
    }
  }

  // Get the transformation matrix
  Matrix
  Object::trans()
  {
    return Matrix(dmnsn()->trans);
  }

  // Set the transformation matrix
  void
  Object::trans(const Matrix& trans)
  {
    dmnsn()->trans = trans.dmnsn();
  }

  // Intersection list for the line l
  Intersection
  Object::intersection(const Line& l)
  {
    return Intersection((*dmnsn()->intersection_fn)(dmnsn(), l.dmnsn()));
  }

  // Whether the point `point' is inside the object
  bool
  Object::inside(const Vector& point)
  {
    return (*dmnsn()->inside_fn)(dmnsn(), point.dmnsn());
  }

  // Return the wrapped object
  dmnsn_object*
  Object::dmnsn()
  {
    if (!m_object) {
      throw Dimension_Error("Attempt to access NULL object.");
    }

    return *m_object;
  }

  Object*
  Object::copy() const
  {
    return new Object(*this);
  }

  // Return a const version of the wrapped canvas
  const dmnsn_object*
  Object::dmnsn() const
  {
    if (!m_object) {
      throw Dimension_Error("Attempt to access NULL object.");
    }

    return *m_object;
  }

  // Protected default no-op constructor
  Object::Object()
    : m_object()
  { }

  // Protected copy constructor
  Object::Object(const Object& object)
    : m_object(object.m_object)
  { }

  // Is m_object unique?
  bool
  Object::unique() const
  {
    return m_object && m_object.unique();
  }

  // Set the wrapped dmnsn_object*
  void
  Object::dmnsn(dmnsn_object* object)
  {
    m_object.reset(new dmnsn_object*(object));
  }

  // Custom object callbacks
  namespace
  {
    dmnsn_intersection *
    intersection_fn(const dmnsn_object *object, dmnsn_line line)
    {
      Custom_Object* cobject = reinterpret_cast<Custom_Object*>(object->ptr);
      return cobject->intersection(Line(line)).release();
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
    dmnsn()->ptr = this;
    dmnsn()->intersection_fn = &intersection_fn;
    dmnsn()->inside_fn       = &inside_fn;
  }

  // Delete the object
  Custom_Object::~Custom_Object()
  {
    if (unique()) {
      dmnsn_delete_object(dmnsn());
    }
  }
}
