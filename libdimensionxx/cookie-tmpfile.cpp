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

// Use a tmpfile() as a buffer for a C++/C I/O interface.

namespace Dimension
{
  namespace
  {
    // Write an input stream completely to a FILE*; this works poorly for
    // console input, which may not have an EOF in the near future
    void
    write_cookie(FILE* file, std::istream& istr)
    {
      const unsigned int bs = 8192; // Bytes to read at once
      unsigned int count, pos;
      char buffer[bs];

      pos = istr.tellg(); // Get the stream's current position
      istr.seekg(0);      // Seek to the beginning
      while (true) {
        // Read the whole stream into `file', `bs' bytes at a time
        istr.read(buffer, bs);
        count = istr.gcount();

        if (count != bs) {
          if (istr.eof()) {
            // We reached EOF; write the last count bytes
            fwrite(buffer, 1, count, file);
            break;
          } else {
            // Some other error
            throw Dimension_Error("Error reading from input stream.");
          }
        }

        // Write the next `bs' bytes
        fwrite(buffer, 1, bs, file);
      }
      fseek(file, pos, SEEK_SET); // Seek to the stream's initial position
    }

    // Read a C++ stream completely from a file
    void
    read_cookie(std::ostream& ostr, FILE* file)
    {
      const unsigned int bs = 8192; // Bytes to read at a time
      unsigned int count, pos;
      char buffer[bs];

      pos = ftell(file); // Get the initial position
      rewind(file);      // Seek to the beginning
      while (true) {
        count = fread(buffer, 1, bs, file);
        if (count != bs) {
          if (feof(file)) {
            // Reached EOF, write the last `count' bytes
            ostr.write(buffer, count);
            break;
          } else {
            // Some other error
            throw Dimension_Error("Error reading from temporary file.");
          }
        }

        // Write the next `bs' bytes
        ostr.write(buffer, bs);
      }
      ostr.seekp(pos); // Seek to the initial position of `file'
    }
  }

  // Make an input FILE_Cookie
  FILE_Cookie::FILE_Cookie(std::istream& istr)
    : m_file(std::tmpfile()), m_istr(&istr), m_ostr(0)
  {
    if (!m_file) {
      throw Dimension_Error("Error opening temporary file for C++/C I/O"
                            " interface.");
    }

    // Write the input stream to the temporary file
    write_cookie(m_file, *m_istr);
  }

  // Make an output FILE_Cookie
  FILE_Cookie::FILE_Cookie(std::ostream& ostr)
    : m_file(std::tmpfile()), m_istr(0), m_ostr(&ostr)
  {
    if (!m_file) {
      throw Dimension_Error("Error opening temporary file for C++/C I/O"
                            " interface.");
    }
  }

  // Make an I/O FILE_Cookie
  FILE_Cookie::FILE_Cookie(std::iostream& iostr)
    : m_file(std::tmpfile()), m_istr(&iostr), m_ostr(&iostr)
  {
    if (!m_file) {
      throw Dimension_Error("Error opening temporary file for C++/C I/O"
                            " interface.");
    }

    // Write the input stream to the temporary file
    write_cookie(m_file, *m_istr);
  }

  // Close the tmpfile, maybe syncing a C++ stream to it first
  FILE_Cookie::~FILE_Cookie() {
    if (is_output()) {
      read_cookie(*m_ostr, m_file);
    }

    std::fclose(m_file);
  }

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
