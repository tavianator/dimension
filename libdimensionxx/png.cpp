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
#include <cstdio>
#include <stdexcept>

namespace Dimension
{
  // PNG_Canvas destructor.  Call write() to write the PNG file if not already
  // written, but catch any exceptions and instead report the error with
  // dmnsn_error() to avoid throwing from a destructor.
  PNG_Canvas::~PNG_Canvas()
  {
    if (m_ostr && !m_written) {
      try {
        write();
      } catch (...) {
        dmnsn_error(SEVERITY_MEDIUM,
                    "Writing canvas to PNG failed in PNG_Canvas destructor.");
      }
    }
  }

  // Write the PNG file.  Uses the FILE_Cookie() interface to make a FILE*
  // corresponding to an std::ostream (including std::ostringstream, etc).
  void PNG_Canvas::write()
  {
    if (m_written) {
      // Does writing a PNG file twice make sense?
      throw Dimension_Error("Attempt to write canvas to PNG twice.");
    }

    if (!m_ostr) {
      // Don't call write() if we're not an output PNG_Canvas...
      throw Dimension_Error("Attempt to write canvas to PNG without an output"
                            " stream.");
    }      

    // Make the C++/C I/O interface
    FILE_Cookie cookie(*m_ostr);

    // Write the PNG file
    if (dmnsn_png_write_canvas(m_canvas, cookie.file())) {
      // The actual write operation failed, for some reason.
      throw Dimension_Error("Writing canvas to PNG failed.");
    }

    m_written = true; // We've written the file now, don't do it again
  }

  // Read a canvas from a PNG file.  Uses the FILE_Cookie() interface to make a
  // FILE* corresponding to an std::istream (including std::istringstream, etc).
  void PNG_Canvas::read()
  {
    if (!m_istr) {
      // read() is private, and only called from the appropriate constructors,
      // so this REALLY shouldn't happen.
      throw Dimension_Error("Attempt to read canvas from PNG without an input"
                            " stream.");
    }      

    // Make the C++/C I/O interface
    FILE_Cookie cookie(*m_istr);

    // Read the canvas from a PNG file
    if (!(m_canvas = dmnsn_png_read_canvas(cookie.file()))) {
      // The read operation failed
      throw Dimension_Error("Reading canvas from PNG failed.");
    }
  }
}