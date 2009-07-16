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

// Camera wrappers.

#ifndef DIMENSIONXX_CAMERAS_HPP
#define DIMENSIONXX_CAMERAS_HPP

namespace Dimension
{
  // Perspective camera
  class Perspective_Camera : public Camera
  {
  public:
    Perspective_Camera();
    // ~Perspective_Camera();

    // Get/set the transformation matrix
    Matrix trans();
    void trans(const Matrix& trans);

    // Shallow-copy the camera
    Camera* copy() const;

  private:
    // Copying prohibited, but used internally
    Perspective_Camera(const Perspective_Camera& camera);
    Perspective_Camera& operator=(const Perspective_Camera&);
  };
}

#endif /* DIMENSIONXX_CAMERAS_HPP */
