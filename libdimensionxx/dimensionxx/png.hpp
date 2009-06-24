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

#ifndef DIMENSIONXX_PNG_HPP
#define DIMENSIONXX_PNG_HPP

// C++ wrapper for libdimension PNG support.  PNG_Canvas derives from Canvas.

#include <istream>
#include <ostream>
#include <memory>

namespace Dimension
{
  // PNG_Canvas handles reading a Canvas from a PNG file, writing one to a PNG
  // file, or both, depending on what type of stream(s) are given to the
  // constructor.
  class PNG_Canvas : public Canvas
  {
  public:
    // Input PNG_Canvas; read the Canvas from istr now
    explicit PNG_Canvas(std::istream& istr);

    // Output PNG_Canvas; write the Canvas to ostr at destruction, or when
    // write() is called.
    PNG_Canvas(unsigned int x, unsigned int y, std::ostream& ostr);

    // I/O PNG_Canvas; read the Canvas from istr now, and write to ostr at
    // destruction or then write() is called.
    PNG_Canvas(std::istream& istr, std::ostream& ostr);

    // Call write() if we're an output PNG_Canvas, but trap any exceptions and
    // report a dmnsn_error() instead.
    virtual ~PNG_Canvas();

    // Write the Canvas to the output stream, throwing a Dimension_Error on
    // error.
    void write();

    // Write the Canvas to the output stream in the background
    Progress write_async();

    // Construct an input or I/O PNG_Canvas in the background
    static Progress read_async(std::istream& istr);
    explicit PNG_Canvas(Progress& progress);
    explicit PNG_Canvas(Progress& progress, std::ostream& ostr);

  protected:
    // In case a derived class needs to set m_canvas after we're constructed
    explicit PNG_Canvas(std::ostream& ostr);

  private:
    // Copying prohibited
    PNG_Canvas(const PNG_Canvas&);
    PNG_Canvas& operator=(const PNG_Canvas&);

    std::istream* m_istr;
    std::ostream* m_ostr;
    bool m_written;

    // Read the Canvas from a PNG file, and throw a Dimension_Error upon
    // failure.
    void read();
  };
}

#endif /* DIMENSIONXX_PNG_HPP */
