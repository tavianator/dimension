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
  // Construct a raytracer
  Raytracer::Raytracer(Scene& scene)
    : m_scene(&scene), m_rendered(false) { }

  // Call render() if we've never rendered the scene
  Raytracer::~Raytracer()
  {
    if (!m_rendered) {
      try {
        render();
      } catch (...) {
        dmnsn_error(SEVERITY_MEDIUM,
                    "Rendering scene failed in Raytracer destructor.");
      }
    }
  }

  // Render the scene
  void Raytracer::render()
  {
    // Raytrace the scene
    if (dmnsn_raytrace_scene(m_scene->dmnsn()) != 0) {
      // The rendering operation failed
      throw Dimension_Error("Raytracing scene failed.");
    }

    m_rendered = true; // Don't render the scene again in the destructor
  }

  // Render a scene in the background
  Progress
  Raytracer::render_async()
  {
    m_rendered = true; // Don't render the scene again in the destructor

    // Start the asynchronous task
    dmnsn_progress *progress = dmnsn_raytrace_scene_async(m_scene->dmnsn());
    if (!progress) {
      throw Dimension_Error("Starting background raytrace failed.");
    }

    // Return the Progress object
    return Progress(progress);
  }
}
