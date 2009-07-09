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
  // Iterator class for scene objects - never invalidated unless removed
  class Scene_Iterator;

  // Base scene class.  Wraps a dmnsn_scene*.
  class Scene
  {
  public:
    typedef Scene_Iterator Iterator;

    // Allocate a dmnsn_scene*
    Scene(const Color& background, Camera& camera, Canvas& canvas);

    // Scene(const Scene& scene);

    // Delete the scene
    ~Scene();

    // Element access
    Color background() const;
    Camera&       camera();
    const Camera& camera() const;
    Canvas&       canvas();
    const Canvas& canvas() const;

    // Object access

    Iterator begin();
    Iterator end();

    void push_object(Object& object);
    void remove_object(Iterator i);

    // Access the wrapped C object.
    dmnsn_scene*       dmnsn();
    const dmnsn_scene* dmnsn() const;

  private:
    // Copy-assignment prohibited
    Scene& operator=(const Scene&);

    std::tr1::shared_ptr<dmnsn_scene*> m_scene;
    std::tr1::shared_ptr<Camera> m_camera;
    std::tr1::shared_ptr<Canvas> m_canvas;
    std::list<std::tr1::shared_ptr<Object> > m_objects;
  };

  class Scene_Iterator
  {
    typedef std::list<std::tr1::shared_ptr<Object> >::iterator Iterator;

  public:
    Scene_Iterator(Iterator i)
      : m_i(i) { }
    // Scene_Iterator(const Scene_Iterator& i);
    // ~Scene_Iterator();

    // Scene_Iterator& operator=(const Scene_Iterator& i);

    Object& operator*() const { return **m_i; }
    Object* operator->() const { return &**m_i; }

    bool operator==(Scene_Iterator i) const { return m_i == i.m_i; }
    bool operator!=(Scene_Iterator i) const { return m_i != i.m_i; }

    Scene_Iterator& operator++()    { ++m_i; return *this; }
    Scene_Iterator  operator++(int) { return Scene_Iterator(m_i++); }
    Scene_Iterator& operator--()    { --m_i; return *this; }
    Scene_Iterator  operator--(int) { return Scene_Iterator(m_i--); }

    Iterator iterator() const { return m_i; }

  private:
    Iterator m_i;
  };
}

#endif /* DIMENSIONXX_SCENE_HPP */
