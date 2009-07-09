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
      m_canvas(new Canvas(canvas))
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

  // An iterator to the beginning of the object list
  Scene::Iterator
  Scene::begin()
  {
    return Iterator(m_objects.begin());
  }

  // An iterator one past the end of the object list
  Scene::Iterator
  Scene::end()
  {
    return Iterator(m_objects.end());
  }

  // Add an object
  void
  Scene::push_object(Object& object)
  {
    m_objects.push_back(std::tr1::shared_ptr<Object>(object.copy()));
    dmnsn_object* cobject = object.dmnsn();
    dmnsn_array_push(dmnsn()->objects, &cobject);
  }

  // Remove an object
  void
  Scene::remove_object(Iterator i)
  {
    // Find it in the dmnsn_array* of objects and remove it
    for (unsigned int j = 0; j < dmnsn_array_size(dmnsn()->objects); ++j) {
      dmnsn_object* cobject;
      dmnsn_array_get(dmnsn()->objects, j, &cobject);
      if (cobject == i->dmnsn()) {
        dmnsn_array_remove(dmnsn()->objects, j);
        break;
      }
    }
    // Remove it from the std::list
    m_objects.erase(i.iterator());
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
