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

// Object wrappers.

#ifndef DIMENSIONXX_OBJECTS_HPP
#define DIMENSIONXX_OBJECTS_HPP

namespace Dimension
{
  // Sphere object
  class Sphere : public Object
  {
  public:
    Sphere();
    ~Sphere();

    // Shallow-copy the sphere
    Object* copy();

  private:
    // Copying prohibited, but used internally
    Sphere(Sphere& sphere);
    Sphere& operator=(const Sphere&);
  };

  // A cube
  class Cube : public Object
  {
  public:
    Cube();
    ~Cube();

    // Shallow-copy the cube
    Object* copy();

  private:
    // Copying prohibited, but used internally
    Cube(Cube& cube);
    Cube& operator=(const Cube&);
  };
}

#endif /* DIMENSIONXX_OBJECTS_HPP */
