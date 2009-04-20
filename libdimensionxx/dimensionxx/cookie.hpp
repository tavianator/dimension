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

#ifndef DIMENSIONXX_COOKIE_HPP
#define DIMENSIONXX_COOKIE_HPP

// Some internal magic to use C FILE* I/O with C++ streams.

#include <istream>
#include <ostream>
#include <cstdio>

namespace Dimension
{
  // Simple RAII class for FILE*'s which interface with a C++ stream.
  class FILE_Cookie
  {
  public:
    FILE_Cookie(std::istream& istr);
    FILE_Cookie(std::ostream& ostr);
    FILE_Cookie(std::iostream& iostr);
    ~FILE_Cookie();

    FILE*       file();
    const FILE* file() const;

    bool is_input() const;
    bool is_output() const;

    std::istream&       istr();
    const std::istream& istr() const;
    std::ostream&       ostr();
    const std::ostream& ostr() const;

  private:
    std::FILE* m_file;
    std::istream* m_istr;
    std::ostream* m_ostr;

    // Copying prohibited
    FILE_Cookie(const FILE_Cookie& cookie);
    FILE_Cookie& operator=(const FILE_Cookie& cookie);
  };
}

#endif /* DIMENSIONXX_COOKIE_HPP */
