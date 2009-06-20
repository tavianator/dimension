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

#ifndef DIMENSIONXX_ARRAY_HPP
#define DIMENSIONXX_ARRAY_HPP

#include <tr1/memory>

// dmnsn_array* wrapper.

namespace Dimension
{
  // Array template class, wraps a dmnsn_array*.  Copying is possible, but
  // copies refer to the same object, which is reference counted.  T must be
  // a POD type.
  template <typename T>
  class Array
  {
  public:
    Array();
    explicit Array(dmnsn_array* array);
    // Array(const Array& a);
    ~Array();

    // Array& operator=(const Array& a);

    // Access the wrapped C object.
    dmnsn_array*       dmnsn()       { return *m_array; }
    const dmnsn_array* dmnsn() const { return *m_array; }

    // Release ownership of the dmnsn_array*, needed for returning a
    // dmnsn_array* from a function.
    dmnsn_array* release();

  private:
    std::tr1::shared_ptr<dmnsn_array*> m_array;
  };

  // A constraint enforcing that T is a POD type by making it part of a union.
  // Taking the address of this function will cause a compile-time failure if
  // T is not a POD type.
  template <typename T>
  void
  POD_constraint()
  {
    union
    {
      T t;
    } constraint;
    static_cast<void>(constraint); // Silence unused variable warning
  }

  template <typename T>
  Array<T>::Array()
    : m_array(new dmnsn_array*(dmnsn_new_array(sizeof(T))))
  {
    void (*constraint)() = &POD_constraint<T>;
    static_cast<void>(constraint); // Silence unused variable warning
  }

  template <typename T>
  Array<T>::Array(dmnsn_array* array)
    : m_array(new dmnsn_array*(array))
  {
    void (*constraint)() = &POD_constraint<T>;
    static_cast<void>(constraint); // Silence unused variable warning
  }

  template <typename T>
  Array<T>::~Array()
  {
    if (m_array.unique()) {
      dmnsn_delete_array(*m_array);
    }
  }

  template <typename T>
  dmnsn_array*
  Array<T>::release()
  {
    if (!m_array.unique()) {
      throw Dimension_Error("Attempting to release non-unique array.");
    }

    dmnsn_array* array = *m_array;
    m_array.reset();
    return array;
  }
}

#endif /* DIMENSIONXX_ARRAY_HPP */
