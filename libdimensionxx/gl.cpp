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
#include <cstdio>

namespace Dimension
{
  GL_Drawer::GL_Drawer(Canvas& canvas)
    : m_canvas(&canvas), m_drawn(false)
  {
    // Optimize the canvas for GL drawing
    dmnsn_gl_optimize_canvas(m_canvas->dmnsn());
  }

  // Draw the canvas if it hasn't been drawn yet
  GL_Drawer::~GL_Drawer()
  {
    if (!m_drawn) {
      try {
        draw();
      } catch (...) {
        dmnsn_error(SEVERITY_MEDIUM,
                    "Drawing canvas to GL failed in GL_Drawer destructor.");
      }
    }
  }

  // Draw the canvas to the current openGL buffer
  void GL_Drawer::draw()
  {
    // Draw to the GL buffer
    if (dmnsn_gl_write_canvas(m_canvas->dmnsn()) != 0) {
      // The drawing operation failed
      throw Dimension_Error("Drawing canvas to GL failed.");
    }

    m_drawn = true; // Don't draw again in destructor
  }

  // No-op
  GL_Reader::GL_Reader() { }

  // Read a canvas from a GL buffer
  Canvas
  GL_Reader::read(unsigned int x0, unsigned int y0,
                  unsigned int width, unsigned int height)
  {
    // Read the canvas from the GL buffer
    dmnsn_canvas* canvas = dmnsn_gl_read_canvas(x0, y0, width, height);
    if (!canvas) {
      // The read operation failed
      throw Dimension_Error("Reading canvas from GL failed.");
    }

    return Canvas(canvas);
  }
}
