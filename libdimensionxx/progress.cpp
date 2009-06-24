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

namespace Dimension
{
  Progress::Progress(dmnsn_progress* progress)
    : m_progress(new dmnsn_progress*(progress))
  { }

  Progress::~Progress()
  {
    if (m_progress.unique()) {
      try {
        dmnsn_finish_progress(dmnsn());
      } catch (...) {
        dmnsn_error(SEVERITY_MEDIUM,
                    "Finishing worker thread failed in Progress destructor.");
      }
    }
  }

  double
  Progress::progress() const
  {
    return dmnsn_get_progress(dmnsn());
  }

  void
  Progress::wait(double progress) const
  {
    dmnsn_wait_progress(dmnsn(), progress);
  }

  void
  Progress::new_element(unsigned int total)
  {
    dmnsn_new_progress_element(dmnsn(), total);
  }

  void
  Progress::increment()
  {
    dmnsn_increment_progress(dmnsn());
  }

  void
  Progress::done()
  {
    dmnsn_progress_done(dmnsn());
  }

  void
  Progress::finish()
  {
    if (m_progress.unique()) {
      dmnsn_finish_progress(dmnsn());
    } else {
      throw Dimension_Error("Attempt to finish non-unique Progress.");
    }
  }

  dmnsn_progress*
  Progress::dmnsn()
  {
    return *m_progress;
  }

  const dmnsn_progress*
  Progress::dmnsn() const
  {
    return *m_progress;
  }
}
