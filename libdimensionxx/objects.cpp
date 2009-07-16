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
  // Create a sphere
  Sphere::Sphere()
    : Object(dmnsn_new_sphere())
  {
    if (!dmnsn()) {
      throw Dimension_Error("Failed to allocate sphere.");
    }
  }

  // Shallow copy a sphere
  Object*
  Sphere::copy() const
  {
    return new Sphere(*this);
  }

  // Protected copy constructor
  Sphere::Sphere(const Sphere& sphere)
    : Object(sphere)
  { }

  // Create a cube
  Cube::Cube()
    : Object(dmnsn_new_cube())
  {
    if (!dmnsn()) {
      throw Dimension_Error("Failed to allocate cube.");
    }
  }

  // Shallow copy a cube
  Object*
  Cube::copy() const
  {
    return new Cube(*this);
  }

  // Protected copy constructor
  Cube::Cube(const Cube& cube)
    : Object(cube)
  { }
}
