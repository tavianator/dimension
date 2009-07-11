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
  // Allocate a dmnsn_scene
  Scene::Scene(const Color& background, Camera& camera, Canvas& canvas)
    : m_scene(new dmnsn_scene*(dmnsn_new_scene())), m_camera(camera.copy()),
      m_canvas(new Canvas(canvas)), m_objects(dmnsn()->objects)
  {
    if (!dmnsn()) {
      throw Dimension_Error("Couldn't allocate scene.");
    }

    dmnsn()->background = background.dmnsn();
    dmnsn()->camera = this->camera().dmnsn();
    dmnsn()->canvas = this->canvas().dmnsn();
  }

  // Delete the scene
  Scene::~Scene()
  {
    if (m_scene.unique()) {
      m_objects.release();
      dmnsn_delete_scene(dmnsn());
    }
  }

  // Element access

  Color
  Scene::background() const
  {
    return Color(dmnsn()->background);
  }

  Camera&
  Scene::camera()
  {
    return *m_camera;
  }

  const Camera&
  Scene::camera() const
  {
    return *m_camera;
  }

  Canvas&
  Scene::canvas()
  {
    return *m_canvas;
  }

  const Canvas&
  Scene::canvas() const
  {
    return *m_canvas;
  }

  Array<Object>&
  Scene::objects()
  {
    return m_objects;
  }

  const Array<Object>&
  Scene::objects() const
  {
    return m_objects;
  }

  // Access the wrapped C object.

  dmnsn_scene*
  Scene::dmnsn()
  {
    return *m_scene;
  }

  const dmnsn_scene*
  Scene::dmnsn() const
  {
    return *m_scene;
  }
}
