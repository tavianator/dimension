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

#ifndef DIMENSIONXX_PROGRESS_HPP
#define DIMENSIONXX_PROGRESS_HPP

#include <tr1/memory> // For tr1::shared_ptr

// dmnsn_canvas* wrapper.

namespace Dimension
{
  // dmnsn_progress* wrapper class to represent an asynchronous worker thread
  class Progress
  {
  public:
    explicit Progress(dmnsn_progress* progress);
    // Progress(const Progress& progress);

    // Finishes the job without throwing
    ~Progress();

    double progress() const;
    void wait(double progress) const;

    void new_element(unsigned int total);
    void increment();
    void done();

    // Wait for job to finish, throwing if the job failed
    void finish();

    // Access the wrapped C object.
    dmnsn_progress*       dmnsn();
    const dmnsn_progress* dmnsn() const;

  private:
    // Copy assignment prohibited
    Progress& operator=(const Progress&);

    std::tr1::shared_ptr<dmnsn_progress*> m_progress;
  };
}

#endif /* DIMENSIONXX_PROGRESS_HPP */
