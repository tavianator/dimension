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
  Pigment::Pigment(dmnsn_pigment* pigment)
    : m_pigment(new dmnsn_pigment*(pigment))
  { }

  Pigment::~Pigment()
  {
    if (unique()) {
      dmnsn_delete_pigment(dmnsn());
    }
  }

  Pigment*
  Pigment::copy() const
  {
    return new Pigment(*this);
  }

  dmnsn_pigment*
  Pigment::dmnsn()
  {
    if (!m_pigment) {
      throw Dimension_Error("Attempt to access NULL pigment.");
    }
    return *m_pigment;
  }

  const dmnsn_pigment*
  Pigment::dmnsn() const
  {
    if (!m_pigment) {
      throw Dimension_Error("Attempt to access NULL pigment.");
    }
    return *m_pigment;
  }

  // Protected no-op constructor
  Pigment::Pigment() { }

  // Shallow copy
  Pigment::Pigment(const Pigment& pigment)
    : m_pigment(pigment.m_pigment)
  {
  }

  bool
  Pigment::unique() const
  {
    return m_pigment && m_pigment.unique();
  }

  Texture::Texture(const Pigment& pigment)
    : m_texture(new dmnsn_texture*(dmnsn_new_texture())),
      m_pigment(pigment.copy())
  {
    dmnsn()->pigment = m_pigment->dmnsn();
  }

  Texture::Texture(dmnsn_texture* texture)
    : m_texture(new dmnsn_texture*(texture)),
      m_pigment(new Pigment(texture->pigment))
  {
  }

  Texture::~Texture()
  {
    if (m_texture.unique()) {
      dmnsn_delete_texture(dmnsn());
    }
  }

  dmnsn_texture*
  Texture::dmnsn()
  {
    return *m_texture;
  }

  const dmnsn_texture*
  Texture::dmnsn() const
  {
    return *m_texture;
  }
}
