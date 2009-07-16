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

// dmnsn_camera* wrapper.

#ifndef DIMENSIONXX_CAMERA_HPP
#define DIMENSIONXX_CAMERA_HPP

namespace Dimension
{
  // Abstract base camera class.  Wraps a dmnsn_camera*.
  class Camera
  {
  public:
    // Delete the camera
    virtual ~Camera();

    // Camera callback
    virtual Line ray(const Canvas& canvas, unsigned int x, unsigned int y);

    // Shallow-copy a derived camera
    virtual Camera* copy() const = 0;

    // Access the wrapped C camera.
    dmnsn_camera*       dmnsn();
    const dmnsn_camera* dmnsn() const;

  protected:
    // No-op
    Camera();
    // Shallow-copy
    Camera(const Camera& camera);
    // Wrap an existing camera
    explicit Camera(dmnsn_camera* camera);

    // Is m_camera unique?
    bool unique() const;

    // Set the wrapped C camera
    void dmnsn(dmnsn_camera* camera);

  private:
    // Copy-assignment prohibited
    Camera& operator=(const Camera&);

    std::tr1::shared_ptr<dmnsn_camera*> m_camera;
  };

  // A custom camera abstract base class, for creating your own camera types
  class Custom_Camera : public Camera
  {
  public:
    Custom_Camera();
    virtual ~Custom_Camera();

    virtual Line ray(const Canvas& canvas, unsigned int x, unsigned int y) = 0;
  };

  // Array_Element specialization
  template <>
  class Array_Element<Camera>
    : public Polymorphic_Array_Element<Camera, dmnsn_camera*>
  {
  public:
    typedef dmnsn_camera* C_Type;

    Array_Element() { }
    Array_Element(Camera& camera)
      : Polymorphic_Array_Element<Camera, dmnsn_camera*>(camera) { }
    Array_Element(C_Type c)
      : Polymorphic_Array_Element<Camera, dmnsn_camera*>(c) { }
    // Array_Element(const Array_Element& ae);
    // ~Array_Element();

    // Array_Element& operator=(const Array_Element& ae);
  };
}

#endif /* DIMENSIONXX_CAMERA_HPP */
