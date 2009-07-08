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

namespace Dimension
{
  // No-op virtual destructor
  Persist_Base::~Persist_Base()
  { }

  // Construct a dmnsn_progress* wrapper
  Progress::Progress(dmnsn_progress* progress)
    : m_progress(new dmnsn_progress*(progress))
  { }

  // Construct a dmnsn_progress* wrapper, with a known persister
  Progress::Progress(dmnsn_progress* progress, const Persister& persister)
    : m_progress(new dmnsn_progress*(progress)), m_persister(persister)
  { }

  // Finish the progress if not yet finished and we are unique
  Progress::~Progress()
  {
    if (m_progress && m_progress.unique()) {
      try {
        finish();
      } catch (...) {
        dmnsn_error(SEVERITY_MEDIUM, "Finishing worker thread failed.");
      }
    }
  }

  // Get the current progress
  double
  Progress::progress() const
  {
    return dmnsn_get_progress(dmnsn());
  }

  // Wait until progress() >= progress
  void
  Progress::wait(double progress) const
  {
    dmnsn_wait_progress(dmnsn(), progress);
  }

  // Start a new level of loop nesting
  void
  Progress::new_element(unsigned int total)
  {
    dmnsn_new_progress_element(dmnsn(), total);
  }

  // Increment the progress
  void
  Progress::increment()
  {
    dmnsn_increment_progress(dmnsn());
  }

  // Immediately finish the progress
  void
  Progress::done()
  {
    dmnsn_done_progress(dmnsn());
  }

  // Wait for progress completion
  void
  Progress::finish()
  {
    if (!m_progress) {
      throw Dimension_Error("Attempt to finish Progress twice.");
    }

    if (!m_progress.unique()) {
      throw Dimension_Error("Attempt to finish non-unique Progress.");
    }

    if (dmnsn_finish_progress(dmnsn()) != 0) {
      throw Dimension_Error("Worker thread failed.");
    }

    m_progress.reset(); // Don't try again
  }

  // Access the set of persisted objects
  Persister&
  Progress::persister()
  {
    return m_persister;
  }

  // Access the underlying dmnsn_progress*

  dmnsn_progress*
  Progress::dmnsn()
  {
    if (!m_progress) {
      throw Dimension_Error("Attempting to access finished array.");
    }

    return *m_progress;
  }

  const dmnsn_progress*
  Progress::dmnsn() const
  {
    if (!m_progress) {
      throw Dimension_Error("Attempting to access finished array.");
    }

    return *m_progress;
  }
}
