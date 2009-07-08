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
  // Allocate the canvas with dmnsn_new_canvas()
  Canvas::Canvas(unsigned int width, unsigned int height)
    : m_canvas(new dmnsn_canvas*(dmnsn_new_canvas(width, height)))
  {
    if (!dmnsn()) {
      throw Dimension_Error("Couldn't allocate canvas.");
    }
  }

  // Wrap an existing dmnsn_canvas*
  Canvas::Canvas(dmnsn_canvas* canvas)
    : m_canvas(new dmnsn_canvas*(canvas)) { }

  // Virtual destructor: delete the canvas with dmnsn_delete_canvas().
  Canvas::~Canvas()
  {
    if (m_canvas && m_canvas.unique()) {
      dmnsn_delete_canvas(dmnsn());
    }
  }

  // Get the width
  unsigned int
  Canvas::width() const
  {
    return dmnsn()->x;
  }

  // Get the height
  unsigned int
  Canvas::height() const
  {
    return dmnsn()->y;
  }

  // Get a particular pixel
  Color
  Canvas::pixel(unsigned int x, unsigned int y) const
  {
    return Color(dmnsn_get_pixel(dmnsn(), x, y));
  }

  // Set a particular pixel
  void
  Canvas::pixel(unsigned int x, unsigned int y, const Color& c)
  {
    dmnsn_set_pixel(dmnsn(), x, y, c.dmnsn());
  }

  // Return the wrapped canvas
  dmnsn_canvas* 
  Canvas::dmnsn()
  {
    return *m_canvas;
  }

  // Return a const version of the wrapped canvas
  const dmnsn_canvas*
  Canvas::dmnsn() const
  {
    return *m_canvas;
  }
}
