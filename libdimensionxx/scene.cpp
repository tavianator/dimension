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
  // Allocate a dmnsn_scene
  Scene::Scene(const Color& background, Camera& camera, Canvas& canvas)
    : m_scene(dmnsn_new_scene()), m_camera(&camera), m_canvas(&canvas)
  {
    if (!m_scene) {
      throw Dimension_Error("Couldn't allocate scene.");
    }

    m_scene->background = background.dmnsn();
    m_scene->camera = camera.dmnsn();
    m_scene->canvas = canvas.dmnsn();
  }

  // Wrap an existing scene
  Scene::Scene(dmnsn_scene* scene)
    : m_scene(scene) { }

  // Delete the scene
  Scene::~Scene()
  {
    dmnsn_delete_scene(m_scene);
  }

  // Element access

  Color
  Scene::background() const
  {
    return Color(m_scene->background);
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

  // Add objects
  void
  Scene::push_object(Object& object)
  {
    dmnsn_object* cobject = object.dmnsn();
    dmnsn_array_push(m_scene->objects, &cobject);
  }

  // Access the wrapped C object.

  dmnsn_scene*
  Scene::dmnsn()
  {
    return m_scene;
  }

  const dmnsn_scene*
  Scene::dmnsn() const
  {
    return m_scene;
  }
}
