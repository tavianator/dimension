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

// dmnsn_camera* wrapper.

#ifndef DIMENSIONXX_CAMERA_HPP
#define DIMENSIONXX_CAMERA_HPP

namespace Dimension
{
  // Abstract base camera class.  Wraps a dmnsn_camera*.
  class Camera
  {
  public:
    // No-op, made pure virtual
    virtual ~Camera() = 0;

    // Camera callback
    virtual Line ray(const Canvas& canvas, unsigned int x, unsigned int y);

    // Access the wrapped C camera.
    dmnsn_camera*       dmnsn();
    const dmnsn_camera* dmnsn() const;

  protected:
    // No-op
    Camera();
    // Wrap an existing camera
    explicit Camera(dmnsn_camera* camera);

    dmnsn_camera* m_camera;

  private:
    // Copying prohibited
    Camera(const Camera&);
    Camera& operator=(const Camera&);
  };

  // A custom camera abstract base class, for creating your own camera types
  class Custom_Camera : public Camera
  {
  public:
    Custom_Camera();
    virtual ~Custom_Camera();

    virtual Line ray(const Canvas& canvas, unsigned int x, unsigned int y) = 0;
  };
}

#endif /* DIMENSIONXX_CAMERA_HPP */
