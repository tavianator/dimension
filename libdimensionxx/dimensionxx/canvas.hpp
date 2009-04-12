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

#ifndef DIMENSIONXX_CANVAS_HPP
#define DIMENSIONXX_CANVAS_HPP

namespace Dimension
{
  class Canvas
  {
  public:
    Canvas(unsigned int x, unsigned int y)
      : m_canvas(dmnsn_new_canvas(x, y)) { }
    virtual ~Canvas();

    Color pixel(unsigned int x, unsigned int y) const
      { return Color(dmnsn_get_pixel(m_canvas, x, y)); }
    void pixel(unsigned int x, unsigned int y, const Color& c)
      { dmnsn_set_pixel(m_canvas, x, y, c.dmnsn()); }

    dmnsn_canvas*       dmnsn()       { return m_canvas; }
    const dmnsn_canvas* dmnsn() const { return m_canvas; }

  private:
    dmnsn_canvas* m_canvas;

    // Copying prohibited
    Canvas(const Canvas&);
    Canvas& operator=(const Canvas&);
  };
}

#endif /* DIMENSIONXX_CANVAS_HPP */
