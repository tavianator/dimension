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

#ifndef DIMENSIONXX_ERROR_HPP
#define DIMENSIONXX_ERROR_HPP

#include <dimension.h>
#include <stdexcept>
#include <string>

namespace Dimension
{
  enum Severity {
    SEVERITY_LOW    = DMNSN_SEVERITY_LOW,
    SEVERITY_MEDIUM = DMNSN_SEVERITY_MEDIUM,
    SEVERITY_HIGH   = DMNSN_SEVERITY_HIGH
  };

  Severity resilience();
  void resilience(Severity resilience);

  class Dimension_Error : public std::runtime_error
  {
  public:
    Dimension_Error(const std::string& str);
  };
}

#endif /* DIMENSIONXX_ERROR_HPP */
