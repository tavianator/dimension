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

#include <tr1/memory> // For tr1::shared_ptr
#include <cstdlib>    // For size_t

// dmnsn_array* wrapper.

namespace Dimension
{
  // RAII scoped read-lock
  class Array_Read_Lock
  {
  public:
    explicit Array_Read_Lock(const dmnsn_array* array)
      : m_array(new const dmnsn_array*(array)) { dmnsn_array_rdlock(*m_array); }
    // Array_Read_Lock(const Array_Read_Lock& lock);
    ~Array_Read_Lock()
    { if (m_array.unique()) { dmnsn_array_unlock(*m_array); } }

  private:
    // Copy assignment prohibited
    Array_Read_Lock& operator=(const Array_Read_Lock&);

    std::tr1::shared_ptr<const dmnsn_array*> m_array;
  };

  // RAII scoped write-lock
  class Array_Write_Lock
  {
  public:
    explicit Array_Write_Lock(dmnsn_array* array)
      : m_array(new dmnsn_array*(array)) { dmnsn_array_wrlock(*m_array); }
    // Array_Write_Lock(const Array_Write_Lock& lock);
    ~Array_Write_Lock()
    { if (m_array.unique()) { dmnsn_array_unlock(*m_array); } }

  private:
    // Copy assignment prohibited
    Array_Write_Lock& operator=(const Array_Write_Lock&);

    std::tr1::shared_ptr<dmnsn_array*> m_array;
  };

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
    ~Array()
    { if (m_array && m_array.unique()) { dmnsn_delete_array(dmnsn()); } }

    // Array& operator=(const Array& a);

    inline T at(std::size_t i) const;
    void set(std::size_t i, T object) { dmnsn_array_set(dmnsn(), i, &object); }

    std::size_t size() const { return dmnsn_array_size(dmnsn()); }
    void resize(std::size_t size) { dmnsn_array_resize(dmnsn(), size); }

    // For manual locking

    Array_Read_Lock read_lock() const { return Array_Read_Lock(dmnsn()); }
    Array_Write_Lock write_lock() { return Array_Write_Lock(dmnsn()); }

    T& operator[](std::size_t i)
    { return *reinterpret_cast<T*>(dmnsn_array_at(dmnsn(), i)); }
    const T& operator[](std::size_t i) const
    { return *reinterpret_cast<const T*>(dmnsn_array_at(dmnsn(), i)); }
    std::size_t size_unlocked() const
    { return dmnsn_array_size_unlocked(dmnsn()); }
    void resize_unlocked(std::size_t size)
    { dmnsn_array_resize_unlocked(dmnsn(), size); }

    // Access the wrapped C object.
    dmnsn_array*       dmnsn();
    const dmnsn_array* dmnsn() const;

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
  inline T
  Array<T>::at(std::size_t i) const
  {
    T ret;
    dmnsn_array_get(dmnsn(), i, &ret);
    return ret;
  }

  template <typename T>
  dmnsn_array*
  Array<T>::dmnsn()
  {
    if (!m_array) {
      throw Dimension_Error("Attempting to access released array.");
    }

    return *m_array;
  }

  template <typename T>
  const dmnsn_array*
  Array<T>::dmnsn() const
  {
    if (!m_array) {
      throw Dimension_Error("Attempting to access released array.");
    }

    return *m_array;
  }

  template <typename T>
  dmnsn_array*
  Array<T>::release()
  {
    dmnsn_array* array = dmnsn();

    if (!m_array.unique()) {
      throw Dimension_Error("Attempting to release non-unique array.");
    } else {
      m_array.reset();
      return array;
    }
  }
}

#endif /* DIMENSIONXX_ARRAY_HPP */
