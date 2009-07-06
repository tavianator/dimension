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

// C++ wrapper for libdimension PNG support.  PNG_Canvas derives from Canvas.

#ifndef DIMENSIONXX_PNG_HPP
#define DIMENSIONXX_PNG_HPP

#include <istream>
#include <ostream>

namespace Dimension
{
  class PNG_Writer
  {
  public:
    PNG_Writer(Canvas& canvas, std::ostream& ostr);
    ~PNG_Writer();

    void write();
    Progress write_async();

  private:
    // Copying prohibited
    PNG_Writer(const PNG_Writer&);
    PNG_Writer& operator=(const PNG_Writer&);

    Canvas* m_canvas;
    std::ostream* m_ostr;
    bool m_written;
  };

  class PNG_Reader
  {
  public:
    PNG_Reader(std::istream& istr);
    // ~PNG_Reader();

    Canvas read();

    Progress read_async();
    static Canvas finish(Progress& progress);

  private:
    // Copying prohibited
    PNG_Reader(const PNG_Reader&);
    PNG_Reader& operator=(const PNG_Reader&);

    std::istream* m_istr;
    bool m_read;
  };
}

#endif /* DIMENSIONXX_PNG_HPP */
