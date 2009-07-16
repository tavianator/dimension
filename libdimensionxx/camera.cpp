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
  // Virtual destructor
  Camera::~Camera()
  {
    if (unique()) {
      dmnsn_delete_camera(dmnsn());
    }
  }

  // Return the result of the dmnsn_camera*'s ray callback
  Line
  Camera::ray(const Canvas& canvas, unsigned int x, unsigned int y)
  {
    return Line((*dmnsn()->ray_fn)(dmnsn(), canvas.dmnsn(), x, y));
  }

  // Return the wrapped camera
  dmnsn_camera*
  Camera::dmnsn()
  {
    if (!m_camera) {
      throw Dimension_Error("Attempt to access NULL camera.");
    }

    return *m_camera;
  }

  // Return a const version of the wrapped canvas
  const dmnsn_camera*
  Camera::dmnsn() const
  {
    if (!m_camera) {
      throw Dimension_Error("Attempt to access NULL camera.");
    }

    return *m_camera;
  }

  // Protected default no-op constructor
  Camera::Camera()
    : m_camera()
  { }

  // Protected copy constructor
  Camera::Camera(Camera& camera)
    : m_camera(camera.m_camera)
  { }

  // Protected manual constructor
  Camera::Camera(dmnsn_camera *camera)
    : m_camera(new dmnsn_camera*(camera))
  { }

  // Is m_camera unique?
  bool
  Camera::unique() const
  {
    return m_camera && m_camera.unique();
  }

  // Set the wrapped dmnsn_camera*
  void
  Camera::dmnsn(dmnsn_camera* camera)
  {
    m_camera.reset(new dmnsn_camera*(camera));
  }

  // Custom camera callbacks
  namespace {
    dmnsn_line
    ray_fn(const dmnsn_camera *camera, const dmnsn_canvas *canvas,
           unsigned int x, unsigned int y)
    {
      Custom_Camera* ccamera = reinterpret_cast<Custom_Camera*>(camera->ptr);
      // Yes the const_cast is ugly, but there's no other way because C++
      // doesn't have `const' constructors.  Luckily const Camera's treat their
      // dmnsn_camera* as a const dmnsn_camera*.
      return ccamera->ray(
        Canvas(const_cast<dmnsn_canvas*>(canvas)), x, y
      ).dmnsn();
    }
  }

  // Initialize a new camera, using member functions as callbacks
  Custom_Camera::Custom_Camera()
    : Camera(dmnsn_new_camera())
  {
    dmnsn()->ptr = this;
    dmnsn()->ray_fn = &ray_fn;
  }

  // Delete the camera
  Custom_Camera::~Custom_Camera()
  {
    if (unique()) {
      dmnsn_delete_camera(dmnsn());
    }
  }
}
