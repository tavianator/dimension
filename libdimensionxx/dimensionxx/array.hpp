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

// dmnsn_array* wrapper.

#ifndef DIMENSIONXX_ARRAY_HPP
#define DIMENSIONXX_ARRAY_HPP

#include <tr1/memory> // For tr1::shared_ptr
#include <cstdlib>    // For size_t
#include <vector>

namespace Dimension
{
  // Class to store POD types, and wrapped dmnsn_* types, including polymorphic
  // types.  The non-specialized version will only handle POD types; specialize
  // it to allow storage of classes in Array's.
  template <typename T>
  class Array_Element
  {
  public:
    typedef T C_Type;

    inline Array_Element();
    inline Array_Element(T t);

    // Specializations should implement this constructor if C_Type differs from
    // T - but it should throw if T is a polymorphic type
    // Array_Element(C_Type c);

    // Array_Element(const Array_Element& ae);
    // ~Array_Element();

    // Array_Element& operator=(const Array_Element& ae);

    C_Type dmnsn() const { return m_t; }
    T& object(C_Type* c) const { return *c; }

  private:
    T m_t;
  };

  // Array template class, wraps a dmnsn_array*.  Copying is possible, but
  // copies refer to the same object, which is reference counted.  T must be
  // a POD type.
  template <typename T>
  class Array
  {
  public:
    inline Array();
    explicit inline Array(dmnsn_array* array);
    // Array(const Array& a);
    ~Array()
      { if (m_array && m_array.unique()) { dmnsn_delete_array(dmnsn()); } }

    // Array& operator=(const Array& a);

    inline T&       operator[](std::size_t i);
    inline const T& operator[](std::size_t i) const;

    std::size_t size() const { return dmnsn_array_size(dmnsn()); }
    inline void resize(std::size_t size);

    inline void push(T& object);
    inline void push(const T& object); // Not valid for polymorphic types
    inline void pop();

    // Access the wrapped C object.
    inline dmnsn_array*       dmnsn();
    inline const dmnsn_array* dmnsn() const;

    // Release ownership of the dmnsn_array*, needed for returning a
    // dmnsn_array* from a function.
    inline dmnsn_array* release();

  private:
    typedef typename Array_Element<T>::C_Type C_Type;

    std::tr1::shared_ptr<dmnsn_array*> m_array;
    std::vector<Array_Element<T> > m_elements;
  };

  // Base class for non-polymorphic wrappers
  template <typename T, typename C>
  class DMNSN_Array_Element
  {
  public:
    typedef C C_Type;

    DMNSN_Array_Element() {
      throw Dimension_Error("Couldn't default-construct an array element.");
    }

    DMNSN_Array_Element(const T& object) : m_object(new T(object)) { }
    DMNSN_Array_Element(C_Type c) : m_object(new T(c)) { }
    // DMNSN_Array_Element(const DMNSN_Array_Element& ae);
    // ~DMNSN_Array_Element();

    // DMNSN_Array_Element& operator=(const DMNSN_Array_Element& ae);

    C_Type dmnsn() const { return m_object->dmnsn(); }
    T& object(C_Type* c) const { return *m_object; }

  private:
    std::tr1::shared_ptr<T> m_object;
  };

  // Base class for polymorphic wrappers
  template <typename T, typename C>
  class Polymorphic_Array_Element
  {
  public:
    typedef C C_Type;

    Polymorphic_Array_Element()
    {
      throw Dimension_Error("Cannot default-construct a polymorphic array"
                            " object.");
    }

    Polymorphic_Array_Element(T& object) : m_object(object.copy()) { }

    Polymorphic_Array_Element(C_Type c)
    {
      throw Dimension_Error("Cannot wrap existing dmnsn_array* elements in"
                            " polymorphic class.");
    }

    // Polymorphic_Array_Element(const Polymorphic_Array_Element& ae);
    // ~Polymorphic_Array_Element();

    // Polymorphic_Array_Element& operator=(const Polymorphic_Array_Element& e);

    C_Type dmnsn() const { return m_object->dmnsn(); }
    T& object(C_Type* c) const { return *m_object; }

  private:
    std::tr1::shared_ptr<T> m_object;
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

  // Array_Element

  template <typename T>
  inline
  Array_Element<T>::Array_Element()
  {
    void (*constraint)() = &POD_constraint<T>;
    static_cast<void>(constraint); // Silence unused variable warning
  }

  template <typename T>
  inline
  Array_Element<T>::Array_Element(T t)
    : m_t(t)
  {
    void (*constraint)() = &POD_constraint<T>;
    static_cast<void>(constraint); // Silence unused variable warning
  }

  // Array constructors

  template <typename T>
  inline
  Array<T>::Array()
    : m_array(new dmnsn_array*(dmnsn_new_array(sizeof(T)))) { }

  template <typename T>
  inline
  Array<T>::Array(dmnsn_array* array)
    : m_array(new dmnsn_array*(array))
  {
    m_elements.reserve(dmnsn_array_size(dmnsn()));
    for (std::size_t i = 0; i < dmnsn_array_size(dmnsn()); ++i) {
      C_Type* c = reinterpret_cast<C_Type*>(dmnsn_array_at(dmnsn(), i));
      m_elements.push_back(Array_Element<T>(*c));
    }
  }

  // Array element access

  template <typename T>
  inline T&
  Array<T>::operator[](std::size_t i)
  {
    if (i >= m_elements.size()) {
      m_elements.resize(i + 1);
    }
    C_Type* c = reinterpret_cast<C_Type*>(dmnsn_array_at(dmnsn(), i));
    return m_elements[i].object(c);
  }

  template <typename T>
  inline const T&
  Array<T>::operator[](std::size_t i) const
  {
    if (i >= m_elements.size()) {
      m_elements.resize(i + 1);
    }
    C_Type* c = reinterpret_cast<C_Type*>(dmnsn_array_at(dmnsn(), i));
    return m_elements[i].object(c);
  }

  template <typename T>
  inline void
  Array<T>::resize(std::size_t size)
  {
    m_elements.resize(size);
    dmnsn_array_resize(dmnsn(), size);
  }

  template <typename T>
  inline void
  Array<T>::push(T& object)
  {
    Array_Element<T> ae(object);
    m_elements.push_back(ae);

    C_Type c = ae.dmnsn();
    dmnsn_array_push(dmnsn(), &c);
  }

  template <typename T>
  inline void
  Array<T>::push(const T& object)
  {
    Array_Element<T> ae(object);
    m_elements.push_back(ae);

    C_Type c = ae.dmnsn();
    dmnsn_array_push(dmnsn(), &c);
  }

  template <typename T>
  inline void
  Array<T>::pop()
  {
    m_elements.pop();
    dmnsn_array_resize(dmnsn_array_size(dmnsn()) - 1);
  }

  // Access the underlying dmnsn_array*

  template <typename T>
  inline dmnsn_array*
  Array<T>::dmnsn()
  {
    if (!m_array) {
      throw Dimension_Error("Attempting to access released array.");
    }

    return *m_array;
  }

  template <typename T>
  inline const dmnsn_array*
  Array<T>::dmnsn() const
  {
    if (!m_array) {
      throw Dimension_Error("Attempting to access released array.");
    }

    return *m_array;
  }

  // Release the dmnsn_array*, if we are the only Array holding it
  template <typename T>
  inline dmnsn_array*
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
