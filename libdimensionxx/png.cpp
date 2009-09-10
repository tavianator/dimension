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
  PNG_Writer::PNG_Writer(Canvas& canvas, std::ostream& ostr)
    : m_canvas(&canvas), m_ostr(&ostr), m_written(false)
  {
    // Optimize the canvas for PNG export
    dmnsn_png_optimize_canvas(m_canvas->dmnsn());
  }

  // PNG_Writer destructor.  Call write() to write the PNG file if not already
  // written, but catch any exceptions and instead report the error with
  // dmnsn_error() to avoid throwing from a destructor.
  PNG_Writer::~PNG_Writer()
  {
    if (!m_written) {
      try {
        write();
      } catch (...) {
        dmnsn_error(SEVERITY_MEDIUM,
                    "Writing canvas to PNG failed in PNG_Writer destructor.");
      }
    }
  }

  // Write the PNG file.  Uses the FILE_Cookie() interface to make a FILE*
  // corresponding to an std::ostream (including std::ostringstream, etc).
  void PNG_Writer::write()
  {
    if (m_written) {
      // Does writing a PNG file twice make sense?
      throw Dimension_Error("Attempt to write canvas to PNG twice.");
    }

    // Make the C++/C I/O interface
    oFILE_Cookie cookie(*m_ostr);

    // Write the PNG file
    if (dmnsn_png_write_canvas(m_canvas->dmnsn(), cookie.file()) != 0) {
      // The actual write operation failed, for some reason
      throw Dimension_Error("Writing canvas to PNG failed.");
    }

    m_written = true; // We've written the file now, don't do it again
  }

  // Write a PNG file in the background
  Progress
  PNG_Writer::write_async()
  {
    if (m_written) {
      // Does writing a PNG file twice make sense?
      throw Dimension_Error("Attempt to write canvas to PNG twice.");
    }

    m_written = true; // We've written the file now, don't do it again

    // Object to persist local variables past function return
    Persister persister;

    // Make the C++/C I/O interface
    FILE_Cookie* cookie = new oFILE_Cookie(*m_ostr);
    persister.persist(cookie);

    // Start the asynchronous task
    dmnsn_progress *progress
      = dmnsn_png_write_canvas_async(m_canvas->dmnsn(), cookie->file());
    if (!progress) {
      throw Dimension_Error("Starting background PNG write failed.");
    }

    // Return the Progress object
    return Progress(progress, persister);
  }

  // Construct a PNG reader
  PNG_Reader::PNG_Reader(std::istream& istr)
    : m_istr(&istr), m_read(false) { }

  // Read a canvas from a PNG file.  Uses the FILE_Cookie() interface to make a
  // FILE* corresponding to an std::istream
  Canvas
  PNG_Reader::read()
  {
    if (m_read) {
      // Does reading a PNG file twice make sense?
      throw Dimension_Error("Attempt to read canvas from PNG twice.");
    }

    // Make the C++/C I/O interface
    iFILE_Cookie cookie(*m_istr);

    // Read the canvas from a PNG file
    dmnsn_canvas* canvas = dmnsn_png_read_canvas(cookie.file());
    if (!canvas) {
      // The read operation failed
      throw Dimension_Error("Reading canvas from PNG failed.");
    }

    // Only set m_read if nothing threw an exception
    Canvas ret(canvas);
    m_read = true;
    return ret;
  }

  // Read a PNG file in the background
  Progress
  PNG_Reader::read_async()
  {
    if (m_read) {
      // Does reading a PNG file twice make sense?
      throw Dimension_Error("Attempt to read canvas from PNG twice.");
    }

    // Don't read again
    m_read = true;

    // Object to persist local variables past function return
    Persister persister;

    // Store a pointer to a dmnsn_canvas* in the persister to later construct
    // the PNG_Writer
    dmnsn_canvas** canvas = new dmnsn_canvas*;
    persister.persist(canvas);

    // Make the C++/C I/O interface
    iFILE_Cookie* cookie = new iFILE_Cookie(*m_istr);
    persister.persist(cookie);

    // Start the asynchronous task
    dmnsn_progress *progress
      = dmnsn_png_read_canvas_async(canvas, cookie->file());
    if (!progress) {
      throw Dimension_Error("Starting background PNG read failed.");
    }

    // Return the Progress object
    return Progress(progress, persister);
  }

  // Construct an input PNG_Writer from a background task
  Canvas
  PNG_Reader::finish(Progress& progress)
  {
    // Will throw if progress is not from a PNG_Writer::read_async call
    dmnsn_canvas** canvas
      = progress.persister().first<dmnsn_canvas*>().persisted();

    try {
      progress.finish();
    } catch (...) {
      dmnsn_delete_canvas(*canvas);
      throw;
    }

    return Canvas(*canvas);
  }
}
