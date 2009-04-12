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
#include <stdio.h>

// The conundrum: libdimension and libdimension-* use C I/O, with FILE*'s.
// We want to use C++ I/O with std::i/ostreams.  Unfortunately, there's no
// standard way to map between them, so we use the nonportable GNU stdio
// extension fopencookie(), which creates a FILE* with custom read/write/seek
// functions.  BSD also has a similar function, funopen(), which we should
// use too.  Failing in all that, we should fall back on a tmpfile() buffer,
// but that would require an fclosecookie() function as well, to copy the
// buffer to the stream potentially.

namespace Dimension
{
  namespace
  {
    // Internal cookie type to hold the C++ streams.
    struct Cookie
    {
    public:
      std::istream* istr;
      std::ostream* ostr;
    };

    // Cookie read function
    ssize_t
    cookie_read(void* cookie, char* buf, size_t size)
    {
      Cookie* streams = reinterpret_cast<Cookie*>(cookie);

      // Do the unformatted read
      streams->istr->read(buf, size);

      if (streams->istr->eof() || streams->istr->good()) {
        return streams->istr->gcount(); // This returns 0 on an immediate EOF
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
      Cookie* streams = reinterpret_cast<Cookie*>(cookie);

      // Do the unformatted write
      streams->ostr->write(buf, size);

      if (streams->ostr->good()) {
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
      Cookie* streams = reinterpret_cast<Cookie*>(cookie);

      if (streams->istr) {
        // If we have an input stream, seek it
        switch (whence) {
        case SEEK_SET:
          streams->istr->seekg(*offset, std::ios::beg);
          break;
        case SEEK_CUR:
          streams->istr->seekg(*offset, std::ios::cur);
          break;
        case SEEK_END:
          streams->istr->seekg(*offset, std::ios::end);
          break;
        }

        if (!streams->istr->good()) {
          // Seek failed
          return 1;
        }
      }

      if (streams->ostr) {
        // If we have an output stream, seek it too
        switch (whence) {
        case SEEK_SET:
          streams->ostr->seekp(*offset, std::ios::beg);
          break;
        case SEEK_CUR:
          streams->ostr->seekp(*offset, std::ios::cur);
          break;
        case SEEK_END:
          streams->ostr->seekp(*offset, std::ios::end);
        }

        if (!streams->ostr->good()) {
          // Seek failed
          return 1;
        }
      }

      // Seek succeeded
      return 0;
    }

    // Cookie delete function
    int
    cookie_close(void* cookie)
    {
      // Free the cookie
      delete reinterpret_cast<Cookie*>(cookie);
    }
  }

  // Make an input FILE*
  std::FILE*
  fcookie(std::istream& istr)
  {
    Cookie* cookie = new Cookie;
    cookie->istr = &istr;
    cookie->ostr = 0;

    cookie_io_functions_t io_funcs;
    io_funcs.read  = &cookie_read;
    io_funcs.write = 0;
    io_funcs.seek  = &cookie_seek;
    io_funcs.close = &cookie_close;

    return fopencookie(reinterpret_cast<void*>(cookie), "r", io_funcs);
  }

  // Make an output FILE*
  std::FILE*
  fcookie(std::ostream& ostr)
  {
    Cookie* cookie = new Cookie;
    cookie->istr = 0;
    cookie->ostr = &ostr;

    cookie_io_functions_t io_funcs;
    io_funcs.read  = 0;
    io_funcs.write = &cookie_write;
    io_funcs.seek  = &cookie_seek;
    io_funcs.close = &cookie_close;

    return fopencookie(reinterpret_cast<void*>(cookie), "w", io_funcs);
  }

  // Make an I/O FILE*
  std::FILE*
  fcookie(std::iostream& iostr)
  {
    Cookie* cookie = new Cookie;
    cookie->istr = &iostr;
    cookie->ostr = &iostr;

    cookie_io_functions_t io_funcs;
    io_funcs.read  = &cookie_read;
    io_funcs.write = &cookie_write;
    io_funcs.seek  = &cookie_seek;
    io_funcs.close = &cookie_close;

    return fopencookie(reinterpret_cast<void*>(cookie), "r+", io_funcs);
  }
}
