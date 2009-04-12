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

// Some internal magic to use C FILE* I/O with C++ streams.  Currently this ties
// us to Linux and glibc, but in the future, this will be portable.

#include <istream>
#include <ostream>
#include <cstdio>

namespace Dimension
{
  std::FILE* fcookie(std::istream& istr);
  std::FILE* fcookie(std::ostream& ostr);
  std::FILE* fcookie(std::iostream& iostr);
}

#endif /* DIMENSIONXX_COOKIE_HPP */
