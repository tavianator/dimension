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

// Use a tmpfile as a buffer for a C++/C I/O interface.

namespace Dimension
{
  namespace
  {
    void
    write_cookie(FILE* file, std::istream& istr)
    {
      unsigned int count, pos;
      const unsigned int bs = 8192;
      char buffer[bs];

      pos = istr.tellg();
      istr.seekg(0);
      while (true) {
        istr.read(buffer, bs);
        count = istr.gcount();

        if (count != bs) {
          if (istr.eof()) {
            fwrite(buffer, 1, count, file);
            break;
          } else {
            throw Dimension_Error("Error reading from input stream.");
          }
        }

        fwrite(buffer, 1, bs, file);
      }
      fseek(file, pos, SEEK_SET);
    }

    void
    read_cookie(std::ostream& ostr, FILE* file)
    {
      unsigned int count, pos;
      const unsigned int bs = 8192;
      char buffer[bs];

      pos = ftell(file);
      rewind(file);
      while (true) {
        count = fread(buffer, 1, bs, file);
        if (count != bs) {
          if (feof(file)) {
            ostr.write(buffer, count);
            break;
          } else {
            throw Dimension_Error("Error reading from temporary file.");
          }
        }

        ostr.write(buffer, bs);
      }
      ostr.seekp(pos);
    }
  }

  // Make an input FILE_Cookie
  FILE_Cookie::FILE_Cookie(std::istream& istr)
    : m_file(tmpfile()), m_istr(&istr), m_ostr(0)
  {
    if (!m_file) {
      throw Dimension_Error("Error opening temporary file for C++/C I/O"
                            " interface.");
    }

    write_cookie(m_file, *m_istr);
  }

  // Make an output FILE_Cookie
  FILE_Cookie::FILE_Cookie(std::ostream& ostr)
    : m_file(tmpfile()), m_istr(0), m_ostr(&ostr)
  {
    if (!m_file) {
      throw Dimension_Error("Error opening temporary file for C++/C I/O"
                            " interface.");
    }
  }

  // Make an I/O FILE_Cookie
  FILE_Cookie::FILE_Cookie(std::iostream& iostr)
    : m_file(tmpfile()), m_istr(&iostr), m_ostr(&iostr)
  {
    if (!m_file) {
      throw Dimension_Error("Error opening temporary file for C++/C I/O"
                            " interface.");
    }

    write_cookie(m_file, *m_istr);
  }

  // Close the tmpfile, maybe syncing a C++ stream to it first
  FILE_Cookie::~FILE_Cookie() {
    if (is_output()) {
      read_cookie(*m_ostr, m_file);
    }

    std::fclose(m_file);
  }

  FILE*       FILE_Cookie::file()       { return m_file; }
  const FILE* FILE_Cookie::file() const { return m_file; }

  bool FILE_Cookie::is_input()  const { return m_istr; }
  bool FILE_Cookie::is_output() const { return m_ostr; }

  std::istream&       FILE_Cookie::istr()       { return *m_istr; }
  const std::istream& FILE_Cookie::istr() const { return *m_istr; }
  std::ostream&       FILE_Cookie::ostr()       { return *m_ostr; }
  const std::ostream& FILE_Cookie::ostr() const { return *m_ostr; }
}
