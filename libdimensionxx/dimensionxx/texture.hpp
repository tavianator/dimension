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

// dmnsn_texture* wrapper.

#ifndef DIMENSIONXX_TEXTURE_HPP
#define DIMENSIONXX_TEXTURE_HPP

namespace Dimension
{
  // Pigment base class.  Wraps a dmnsn_pigment*.
  class Pigment
  {
  public:
    explicit Pigment(dmnsn_pigment* pigment);
    ~Pigment();

    Pigment* copy() const;

    dmnsn_pigment*       dmnsn();
    const dmnsn_pigment* dmnsn() const;

  protected:
    // No-op
    Pigment();
    // Shallow copy
    Pigment(const Pigment& pigment);

    // Is m_pigment unique?
    bool unique() const;

  private:
    // Copy-assignment prohibited
    Pigment& operator=(const Pigment&);

    std::tr1::shared_ptr<dmnsn_pigment*> m_pigment;
  };

  // Texture class.  Wraps a dmnsn_texture*.
  class Texture
  {
  public:
    Texture(const Pigment& pigment);
    explicit Texture(dmnsn_texture* texture);
    // Texture(const Texture& texture);
    ~Texture();

    dmnsn_texture*       dmnsn();
    const dmnsn_texture* dmnsn() const;

  private:
    // Copy-assignment prohibited
    Texture& operator=(const Texture&);

    std::tr1::shared_ptr<dmnsn_texture*> m_texture;
    std::tr1::shared_ptr<Pigment> m_pigment;
  };

  // Array_Element specializations

  template <>
  class Array_Element<Pigment>
    : public Polymorphic_Array_Element<Pigment, dmnsn_pigment*>
  {
  public:
    typedef dmnsn_pigment* C_Type;

    Array_Element() { }
    Array_Element(Pigment& pigment)
      : Polymorphic_Array_Element<Pigment, dmnsn_pigment*>(pigment) { }
    Array_Element(C_Type c)
      : Polymorphic_Array_Element<Pigment, dmnsn_pigment*>(c) { }
    // Array_Element(const Array_Element& ae);
    // ~Array_Element();

    // Array_Element& operator=(const Array_Element& ae);
  };

  template <>
  class Array_Element<Texture>
    : public DMNSN_Array_Element<Texture, dmnsn_texture*>
  {
  public:
    typedef dmnsn_texture* C_Type;

    Array_Element() { }
    Array_Element(Texture& texture)
      : DMNSN_Array_Element<Texture, dmnsn_texture*>(texture) { }
    Array_Element(C_Type c)
      : DMNSN_Array_Element<Texture, dmnsn_texture*>(c) { }
    // Array_Element(const Array_Element& ae);
    // ~Array_Element();

    // Array_Element& operator=(const Array_Element& ae);
  };
}

#endif /* DIMENSIONXX_TEXTURE_HPP */
