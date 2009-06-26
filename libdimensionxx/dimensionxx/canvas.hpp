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

// dmnsn_canvas* wrapper.

#ifndef DIMENSIONXX_CANVAS_HPP
#define DIMENSIONXX_CANVAS_HPP

namespace Dimension
{
  // Base canvas class.  Wraps a dmnsn_canvas*.
  class Canvas
  {
  public:
    // Allocate a dmnsn_canvas specified width and height.
    Canvas(unsigned int width, unsigned int height);

    // Wrap an existing canvas.
    explicit Canvas(dmnsn_canvas* canvas) : m_canvas(canvas) { }

    // Delete the canvas.  (dmnsn_delete_canvas(m_canvas).)
    virtual ~Canvas();

    // Get the width and height.
    unsigned int width() const;
    unsigned int height() const;

    // Get and set a pixel
    Color pixel(unsigned int x, unsigned int y) const;
    void pixel(unsigned int x, unsigned int y, const Color& c);

    // Access the wrapped C object.
    dmnsn_canvas*       dmnsn();
    const dmnsn_canvas* dmnsn() const;

  protected:
    // Derived classes may want to set m_canvas later.  Set it to NULL now, so
    // that the destructor can still dmnsn_delete_canvas it.
    Canvas();

    dmnsn_canvas* m_canvas;

  private:
    // Copying prohibited
    Canvas(const Canvas&);
    Canvas& operator=(const Canvas&);
  };
}

#endif /* DIMENSIONXX_CANVAS_HPP */
