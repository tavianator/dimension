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
  // FILE_Cookie pure virtual destructor: close the file
  FILE_Cookie::~FILE_Cookie() { std::fclose(m_file); }

  namespace
  {
    // Cookie read function
    ssize_t
    cookie_read(void* cookie, char* buf, size_t size)
    {
      FILE_Cookie*  fcookie  = reinterpret_cast<FILE_Cookie*>(cookie);
      iFILE_Cookie& ifcookie = dynamic_cast<iFILE_Cookie&>(*fcookie);

      // Do the unformatted read
      ifcookie.istr().read(buf, size);

      if (ifcookie.istr().eof() || ifcookie.istr().good()) {
        return ifcookie.istr().gcount(); // This returns 0 on an immediate EOF
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
      FILE_Cookie*  fcookie  = reinterpret_cast<FILE_Cookie*>(cookie);
      oFILE_Cookie& ofcookie = dynamic_cast<oFILE_Cookie&>(*fcookie);

      // Do the unformatted write
      ofcookie.ostr().write(buf, size);

      if (ofcookie.ostr().good()) {
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
      FILE_Cookie*  fcookie  = reinterpret_cast<FILE_Cookie*>(cookie);
      iFILE_Cookie* ifcookie = dynamic_cast<iFILE_Cookie*>(fcookie);
      oFILE_Cookie* ofcookie = dynamic_cast<oFILE_Cookie*>(fcookie);

      if (ifcookie) {
        // If we have an input stream, seek it
        switch (whence) {
        case SEEK_SET:
          ifcookie->istr().seekg(*offset, std::ios::beg);
          break;
        case SEEK_CUR:
          ifcookie->istr().seekg(*offset, std::ios::cur);
          break;
        case SEEK_END:
          ifcookie->istr().seekg(*offset, std::ios::end);
          break;
        }

        if (!ifcookie->istr().good()) {
          // Seek failed
          return 1;
        }
      }

      if (ofcookie) {
        // If we have an output stream, seek it
        switch (whence) {
        case SEEK_SET:
          ofcookie->ostr().seekp(*offset, std::ios::beg);
          break;
        case SEEK_CUR:
          ofcookie->ostr().seekp(*offset, std::ios::cur);
          break;
        case SEEK_END:
          ofcookie->ostr().seekp(*offset, std::ios::end);
        }

        if (!ofcookie->ostr().good()) {
          // Seek failed
          return 1;
        }
      }

      // Seek succeeded
      return 0;
    }
  }

  // Make an input FILE_Cookie
  iFILE_Cookie::iFILE_Cookie(std::istream& istr)
    : m_istr(&istr)
  {
    cookie_io_functions_t io_funcs;
    io_funcs.read  = &cookie_read;
    io_funcs.write = 0;
    io_funcs.seek  = &cookie_seek;
    io_funcs.close = 0;

    // Set the FILE*
    file(fopencookie(reinterpret_cast<void*>(this), "r", io_funcs));
  }

  // Make an output FILE_Cookie
  oFILE_Cookie::oFILE_Cookie(std::ostream& ostr)
    : m_ostr(&ostr)
  {
    cookie_io_functions_t io_funcs;
    io_funcs.read  = 0;
    io_funcs.write = &cookie_write;
    io_funcs.seek  = &cookie_seek;
    io_funcs.close = 0;

    // Set the FILE*
    file(fopencookie(reinterpret_cast<void*>(this), "w", io_funcs));
  }

  // No-op oFILE_Cookie destructor
  oFILE_Cookie::~oFILE_Cookie() { }

  // Make an I/O FILE_Cookie
  ioFILE_Cookie::ioFILE_Cookie(std::iostream& iostr)
    : iFILE_Cookie(iostr, 0), oFILE_Cookie(iostr, 0)
  {
    cookie_io_functions_t io_funcs;
    io_funcs.read  = &cookie_read;
    io_funcs.write = &cookie_write;
    io_funcs.seek  = &cookie_seek;
    io_funcs.close = 0;

    file(fopencookie(reinterpret_cast<void*>(this), "r+", io_funcs));
  }
}
