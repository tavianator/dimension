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

// C++ wrapper for libdimension raytracing

#ifndef DIMENSIONXX_RAYTRACE_HPP
#define DIMENSIONXX_RAYTRACE_HPP

#include <istream>
#include <ostream>

namespace Dimension
{
  class Raytracer
  {
  public:
    Raytracer(Scene& scene);
    ~Raytracer();

    // Render the scene
    void render();
    Progress render_async();

  private:
    // Copying prohibited
    Raytracer(const Raytracer&);
    Raytracer& operator=(const Raytracer&);

    Scene* m_scene;
    bool m_rendered;
  };
}

#endif /* DIMENSIONXX_RAYTRACE_HPP */
