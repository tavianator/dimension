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

namespace Dimension
{
  namespace
  {
    struct Cookie
    {
    public:
      std::istream* istr;
      std::ostream* ostr;
    };

    ssize_t
    cookie_read(void* cookie, char* buf, size_t size)
    {
      Cookie* streams = reinterpret_cast<Cookie*>(cookie);

      streams->istr->read(buf, size);

      if (streams->istr->eof()) {
        return streams->istr->gcount();
      } else if (!streams->istr->good()) {
        return -1;
      } else {
        return streams->istr->gcount();
      }
    }

    ssize_t
    cookie_write(void* cookie, const char* buf, size_t size)
    {
      Cookie* streams = reinterpret_cast<Cookie*>(cookie);

      streams->ostr->write(buf, size);

      if (!streams->ostr->good()) {
        return -1;
      } else {
        return size;
      }
    }

    int
    cookie_seek(void* cookie, off64_t* offset, int whence)
    {
      Cookie* streams = reinterpret_cast<Cookie*>(cookie);
      bool success = true;

      if (streams->istr) {
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
          success = false;
        }
      }

      if (streams->ostr) {
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
          success = false;
        }
      }

      return !success;
    }

    int
    cookie_close(void* cookie)
    {
      delete reinterpret_cast<Cookie*>(cookie);
    }
  }

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
