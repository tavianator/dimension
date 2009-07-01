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
  // Create a sphere
  Sphere::Sphere()
    : Object(dmnsn_new_sphere())
  {
    if (!m_object) {
      throw Dimension_Error("Failed to allocate sphere.");
    }
  }

  // Delete a sphere
  Sphere::~Sphere()
  {
    dmnsn_delete_sphere(m_object);
  }

  // Create a cube
  Cube::Cube()
    : Object(dmnsn_new_cube())
  {
    if (!m_object) {
      throw Dimension_Error("Failed to allocate sphere.");
    }
  }

  // Delete a sphere
  Cube::~Cube()
  {
    dmnsn_delete_cube(m_object);
  }
}
