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

// dmnsn_scene* wrapper.

#ifndef DIMENSIONXX_SCENE_HPP
#define DIMENSIONXX_SCENE_HPP

namespace Dimension
{
  // Base scene class.  Wraps a dmnsn_scene*.
  class Scene
  {
  public:
    // Allocate a dmnsn_scene
    Scene(const Color& background, Camera& camera, Canvas& canvas);
    // Wrap an existing scene
    explicit Scene(dmnsn_scene* scene);
    // Delete the scene
    ~Scene();

    // Element access
    Color background() const;
    Camera&       camera();
    const Camera& camera() const;
    Canvas&       canvas();
    const Canvas& canvas() const;

    // Add objects
    void push_object(Object& object);

    // Render it!
    void raytrace();
    Progress raytrace_async();

    // Access the wrapped C object.
    dmnsn_scene*       dmnsn();
    const dmnsn_scene* dmnsn() const;

  private:
    // Copying prohibited
    Scene(const Scene&);
    Scene& operator=(const Scene&);

    dmnsn_scene* m_scene;
    Camera* m_camera;
    Canvas* m_canvas;
  };
}

#endif /* DIMENSIONXX_SCENE_HPP */
