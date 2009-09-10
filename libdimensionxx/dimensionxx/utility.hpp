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

// Template utilities

#ifndef DIMENSIONXX_UTILITY_HPP
#define DIMENSIONXX_UTILITY_HPP

namespace Dimension
{
  // A constraint enforcing that T is a POD type by making it part of a union.
  // Taking the address of this function will cause a compile-time failure if
  // T is not a POD type.
  template <typename T>
  void POD_constraint();

  // POD constraint implementation
  template <typename T>
  void
  POD_constraint()
  {
    union
    {
      T t;
    } constraint;
    static_cast<void>(constraint); // Silence unused variable warning
  }
}

#endif // DIMENSIONXX_UTILITY_HPP
