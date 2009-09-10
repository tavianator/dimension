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
#include <stdio.h>

// Use a tmpfile() as a buffer for a C++/C I/O interface.

namespace Dimension
{
  // FILE_Cookie pure virtual destructor
  FILE_Cookie::~FILE_Cookie() {
    fclose(file());
  }

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
  iFILE_Cookie::iFILE_Cookie(std::istream& istr)
    : m_istr(&istr)
  {
    FILE* tmp = std::tmpfile();
    if (!tmp) {
      throw Dimension_Error("Error opening temporary file for C++/C I/O"
                            " interface.");
    }

    // Write the input stream to the temporary file
    write_cookie(tmp, *m_istr);

    // Set the FILE*
    file(tmp);
  }

  // No-op iFILE_Cookie destructor
  iFILE_Cookie::~iFILE_Cookie() { }

  // Make an output FILE_Cookie
  oFILE_Cookie::oFILE_Cookie(std::ostream& ostr)
    : m_ostr(&ostr)
  {
    FILE* tmp = std::tmpfile();
    if (!tmp) {
      throw Dimension_Error("Error opening temporary file for C++/C I/O"
                            " interface.");
    }

    // Set the FILE*
    file(tmp);
  }

  // Write the temporary file to the output stream
  oFILE_Cookie::~oFILE_Cookie()
  {
    read_cookie(ostr(), file());
  }

  // Make an I/O FILE_Cookie
  ioFILE_Cookie::ioFILE_Cookie(std::iostream& iostr)
    : iFILE_Cookie(iostr, 0), oFILE_Cookie(iostr, 0)
  {
    FILE* tmp = std::tmpfile();
    if (!tmp) {
      throw Dimension_Error("Error opening temporary file for C++/C I/O"
                            " interface.");
    }

    // Write the input stream to the temporary file
    write_cookie(tmp, istr());

    // Set the FILE*
    file(tmp);
  }

  // No-op ioFILE_Cookie destructor
  ioFILE_Cookie::~ioFILE_Cookie() { }
}
