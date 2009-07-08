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

// C++ wrapper for libdimension GL support

#ifndef DIMENSIONXX_GL_HPP
#define DIMENSIONXX_GL_HPP

#include <istream>
#include <ostream>

namespace Dimension
{
  class GL_Drawer
  {
  public:
    GL_Drawer(Canvas& canvas);
    ~GL_Drawer();

    void draw();

  private:
    // Copying prohibited
    GL_Drawer(const GL_Drawer&);
    GL_Drawer& operator=(const GL_Drawer&);

    Canvas* m_canvas;
    bool m_drawn;
  };

  class GL_Reader
  {
  public:
    GL_Reader();
    // ~GL_Reader();

    Canvas read(unsigned int x0, unsigned int y0,
                unsigned int width, unsigned int height);

  private:
    // Copying prohibited
    GL_Reader(const GL_Reader&);
    GL_Reader& operator=(const GL_Reader&);
  };
}

#endif /* DIMENSIONXX_GL_HPP */
