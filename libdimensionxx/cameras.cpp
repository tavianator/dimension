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
  // Create a perspective camera
  Perspective_Camera::Perspective_Camera(const Matrix& trans)
    : Camera(dmnsn_new_perspective_camera(trans.dmnsn()))
  {
    if (!dmnsn()) {
      throw Dimension_Error("Failed to allocate perspective camera.");
    }
  }

  // Delete a perspective camera
  Perspective_Camera::~Perspective_Camera()
  {
    if (unique()) {
      dmnsn_delete_perspective_camera(dmnsn());
    }
  }

  Matrix
  Perspective_Camera::trans()
  {
    return Matrix(dmnsn_get_perspective_camera_trans(dmnsn()));
  }

  void
  Perspective_Camera::trans(const Matrix& trans)
  {
    dmnsn_set_perspective_camera_trans(dmnsn(), trans.dmnsn());
  }

  Camera*
  Perspective_Camera::copy() const
  {
    return new Perspective_Camera(*this);
  }

  Perspective_Camera::Perspective_Camera(const Perspective_Camera& camera)
    : Camera(camera)
  { }
}
