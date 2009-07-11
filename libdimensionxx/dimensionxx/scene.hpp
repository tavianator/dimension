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

// dmnsn_scene* wrapper.

#ifndef DIMENSIONXX_SCENE_HPP
#define DIMENSIONXX_SCENE_HPP

namespace Dimension
{
  // Base scene class.  Wraps a dmnsn_scene*.
  class Scene
  {
  public:
    // Allocate a dmnsn_scene*
    Scene(const Color& background, Camera& camera, Canvas& canvas);

    // Scene(const Scene& scene);

    // Delete the scene
    ~Scene();

    // Element access
    Color                background() const;
    Camera&              camera();
    const Camera&        camera() const;
    Canvas&              canvas();
    const Canvas&        canvas() const;
    Array<Object>&       objects();
    const Array<Object>& objects() const;

    // Access the wrapped C object.
    dmnsn_scene*       dmnsn();
    const dmnsn_scene* dmnsn() const;

  private:
    // Copy-assignment prohibited
    Scene& operator=(const Scene&);

    std::tr1::shared_ptr<dmnsn_scene*> m_scene;
    std::tr1::shared_ptr<Camera> m_camera;
    std::tr1::shared_ptr<Canvas> m_canvas;
    Array<Object> m_objects;
  };
}

#endif /* DIMENSIONXX_SCENE_HPP */
