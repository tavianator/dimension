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

// Some internal magic to use C FILE* I/O with C++ streams.

#ifndef DIMENSIONXX_COOKIE_HPP
#define DIMENSIONXX_COOKIE_HPP

#include <istream>
#include <ostream>
#include <cstdio>

namespace Dimension
{
  // Simple RAII classes for FILE*'s which interface with a C++ stream.

  class FILE_Cookie
  {
  public:
    // Destructor made pure virtual
    virtual ~FILE_Cookie() = 0;

    // Get the magic FILE*
    FILE*       file()       { return m_file; }
    const FILE* file() const { return m_file; }

  protected:
    FILE_Cookie() { }

    // Set the underlying FILE*
    void file(FILE* file) { m_file = file; }

  private:
    std::FILE* m_file;

    // Copying prohibited
    FILE_Cookie(const FILE_Cookie& cookie);
    FILE_Cookie& operator=(const FILE_Cookie& cookie);
  };

  class iFILE_Cookie : public virtual FILE_Cookie
  {
  public:
    iFILE_Cookie(std::istream& istr);
    virtual ~iFILE_Cookie();

    // Get the C++ streams
    std::istream&       istr()       { return *m_istr; }
    const std::istream& istr() const { return *m_istr; }

  protected:
    // Just set the istream without initializing the FILE*
    iFILE_Cookie(std::istream& istr, int) : m_istr(&istr) { }

  private:
    std::istream* m_istr;
  };

  class oFILE_Cookie : public virtual FILE_Cookie
  {
  public:
    oFILE_Cookie(std::ostream& ostr);
    virtual ~oFILE_Cookie();

    // Get the C++ streams
    std::ostream&       ostr()       { return *m_ostr; }
    const std::ostream& ostr() const { return *m_ostr; }

  protected:
    // Just set the istream without initializing the FILE*
    oFILE_Cookie(std::ostream& ostr, int) : m_ostr(&ostr) { }

  private:
    std::ostream* m_ostr;
  };

  class ioFILE_Cookie : public iFILE_Cookie, public oFILE_Cookie
  {
  public:
    ioFILE_Cookie(std::iostream& iostr);
    virtual ~ioFILE_Cookie();
  };
}

#endif /* DIMENSIONXX_COOKIE_HPP */
