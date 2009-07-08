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

// dmnsn_object* wrapper.

#ifndef DIMENSIONXX_OBJECT_HPP
#define DIMENSIONXX_OBJECT_HPP

namespace Dimension
{
  // Abstract base object class.  Wraps a dmnsn_object*.
  class Object
  {
  public:
    // No-op
    virtual ~Object();

    // Get/set the transformation matrix
    Matrix trans();
    void trans(const Matrix& trans);

    // Object callbacks
    virtual Array<double> intersections(const Line& l);
    virtual bool inside(const Vector& point);

    // Shallow-copy a derived object
    virtual Object* copy() const = 0;

    // Access the wrapped C object
    dmnsn_object*       dmnsn();
    const dmnsn_object* dmnsn() const;

  protected:
    // No-op
    Object();
    // Shallow copy
    Object(const Object& object);
    // Wrap an existing object.
    explicit Object(dmnsn_object* object);

    // Is m_object unique?
    bool unique() const;

    // Set the wrapped object
    void dmnsn(dmnsn_object* object);

  private:
    // Copy-assignment prohibited
    Object& operator=(const Object&);

    std::tr1::shared_ptr<dmnsn_object*> m_object;
  };

  // A custom object abstract base class, for creating your own object types
  class Custom_Object : public Object
  {
  public:
    Custom_Object();
    virtual ~Custom_Object();

    virtual Array<double> intersections(const Line& l) = 0;
    virtual bool inside(const Vector& point) = 0;
  };
}

#endif /* DIMENSIONXX_OBJECT_HPP */
