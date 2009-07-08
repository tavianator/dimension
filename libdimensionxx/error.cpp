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
#include <stdexcept>
#include <string>

namespace Dimension
{
  // Get the resilience, thread-safely, with dmnsn_get_resilience().
  Severity
  resilience()
  {
    return static_cast<Severity>(dmnsn_get_resilience());
  }

  // Set the resilience, thread-safely, with dmnsn_set_resilience().
  void
  resilience(Severity resilience)
  {
    dmnsn_set_resilience(static_cast<dmnsn_severity>(resilience));
  }

  // Dimension_Error constructor
  Dimension_Error::Dimension_Error(const std::string& str)
    : std::runtime_error(str) { }
}
