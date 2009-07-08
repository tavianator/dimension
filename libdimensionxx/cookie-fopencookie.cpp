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

#ifndef _GNU_SOURCE
  // For fopencookie()
  #define _GNU_SOURCE
#endif
#include <stdio.h>

// The conundrum: libdimension uses C I/O, with FILE*'s. We want to use C++ I/O
// with std::i/ostreams. If present, we use the nonportable GNU stdio extension
// fopencookie(), which creates a FILE* with custom read/write/seek functions.
// BSD also has a similar function, funopen() which we should use too.  Failing
// in all that, fall back on a tmpfile() buffer (see cookie-tmpfile.cpp).

namespace Dimension
{
  namespace
  {
    // Cookie read function
    ssize_t
    cookie_read(void* cookie, char* buf, size_t size)
    {
      FILE_Cookie* streams = reinterpret_cast<FILE_Cookie*>(cookie);

      // Do the unformatted read
      streams->istr().read(buf, size);

      if (streams->istr().eof() || streams->istr().good()) {
        return streams->istr().gcount(); // This returns 0 on an immediate EOF
                                         // for us.
      } else {
        // Some non-EOF error
        return -1;
      }
    }

    // Cookie write function
    ssize_t
    cookie_write(void* cookie, const char* buf, size_t size)
    {
      FILE_Cookie* streams = reinterpret_cast<FILE_Cookie*>(cookie);

      // Do the unformatted write
      streams->ostr().write(buf, size);

      if (streams->ostr().good()) {
        // Write operation succeeded, so we must've written size bytes
        return size;
      } else {
        // Write operation failed
        return -1;
      }
    }

    // Cookie seek function
    int
    cookie_seek(void* cookie, off64_t* offset, int whence)
    {
      FILE_Cookie* streams = reinterpret_cast<FILE_Cookie*>(cookie);

      if (streams->is_input()) {
        // If we have an input stream, seek it
        switch (whence) {
        case SEEK_SET:
          streams->istr().seekg(*offset, std::ios::beg);
          break;
        case SEEK_CUR:
          streams->istr().seekg(*offset, std::ios::cur);
          break;
        case SEEK_END:
          streams->istr().seekg(*offset, std::ios::end);
          break;
        }

        if (!streams->istr().good()) {
          // Seek failed
          return 1;
        }
      }

      if (streams->is_output()) {
        // If we have an output stream, seek it
        switch (whence) {
        case SEEK_SET:
          streams->ostr().seekp(*offset, std::ios::beg);
          break;
        case SEEK_CUR:
          streams->ostr().seekp(*offset, std::ios::cur);
          break;
        case SEEK_END:
          streams->ostr().seekp(*offset, std::ios::end);
        }

        if (!streams->ostr().good()) {
          // Seek failed
          return 1;
        }
      }

      // Seek succeeded
      return 0;
    }
  }

  // Make an input FILE_Cookie
  FILE_Cookie::FILE_Cookie(std::istream& istr)
    : m_istr(&istr), m_ostr(0)
  {
    cookie_io_functions_t io_funcs;
    io_funcs.read  = &cookie_read;
    io_funcs.write = 0;
    io_funcs.seek  = &cookie_seek;
    io_funcs.close = 0;

    m_file = fopencookie(reinterpret_cast<void*>(this), "r", io_funcs);
  }

  // Make an output FILE_Cookie
  FILE_Cookie::FILE_Cookie(std::ostream& ostr)
    : m_istr(0), m_ostr(&ostr)
  {
    cookie_io_functions_t io_funcs;
    io_funcs.read  = 0;
    io_funcs.write = &cookie_write;
    io_funcs.seek  = &cookie_seek;
    io_funcs.close = 0;

    m_file = fopencookie(reinterpret_cast<void*>(this), "w", io_funcs);
  }

  // Make an I/O FILE_Cookie
  FILE_Cookie::FILE_Cookie(std::iostream& iostr)
    : m_istr(&iostr), m_ostr(&iostr)
  {
    cookie_io_functions_t io_funcs;
    io_funcs.read  = &cookie_read;
    io_funcs.write = &cookie_write;
    io_funcs.seek  = &cookie_seek;
    io_funcs.close = 0;

    m_file = fopencookie(reinterpret_cast<void*>(this), "r+", io_funcs);
  }

  // Close the file
  FILE_Cookie::~FILE_Cookie() { std::fclose(m_file); }

  // Get the FILE*
  FILE*       FILE_Cookie::file()       { return m_file; }
  const FILE* FILE_Cookie::file() const { return m_file; }

  bool FILE_Cookie::is_input()  const { return m_istr; }
  bool FILE_Cookie::is_output() const { return m_ostr; }

  // Get the C++ streams

  std::istream&
  FILE_Cookie::istr()
  {
    if (is_input()) {
      return *m_istr;
    } else {
      throw Dimension_Error("Attempted to get input stream from non-input"
                            " FILE_Cookie.");
    }
  }

  const std::istream&
  FILE_Cookie::istr() const
  {
    if (is_input()) {
      return *m_istr;
    } else {
      throw Dimension_Error("Attempted to get input stream from non-input"
                            " FILE_Cookie.");
    }
  }

  std::ostream&
  FILE_Cookie::ostr()
  {
    if (is_output()) {
      return *m_ostr;
    } else {
      throw Dimension_Error("Attempted to get output stream from non-input"
                            " FILE_Cookie.");
    }
  }

  const std::ostream& FILE_Cookie::ostr() const
  {
    if (is_output()) {
      return *m_ostr;
    } else {
      throw Dimension_Error("Attempted to get output stream from non-input"
                            " FILE_Cookie.");
    }
  }
}
