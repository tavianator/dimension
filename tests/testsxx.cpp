/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * The Dimension Test Suite is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License as *
 * published by the Free Software Foundation; either version 3 of the    *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Test Suite is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "testsxx.hpp"

namespace Dimension
{
  Display::Display(const Canvas& canvas)
    : m_display(dmnsn_new_display(canvas.dmnsn()))
  {
    if (!m_display) {
      throw Dimension_Error("Couldn't create display.");
    }
  }

  Display::~Display()
  {
    dmnsn_delete_display(m_display);
  }

  void
  Display::flush()
  {
    dmnsn_display_frame(m_display);
  }

  // Print a progress bar of the progress of `progress'
  std::ostream&
  operator<<(std::ostream& ostr, const Dimension::Progress& progress)
  {
    const unsigned int increments = 32;

    ostr << "|" << std::flush;
    for (unsigned int i = 0; i < increments; ++i) {
      progress.wait(static_cast<double>(i + 1)/increments);
      ostr << "=" << std::flush;
    }
    return ostr << "|" << std::flush;
  }
}