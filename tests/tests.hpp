/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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

#ifndef TESTSXX_HPP
#define TESTSXX_HPP

#include "../libdimensionxx/dimensionxx.hpp"
#include <iostream>
#include <GL/glx.h>
#include <GL/gl.h>

namespace Dimension
{
  namespace Tests
  {
    // Helper to return a basic scene
    Scene default_scene();

    // Display abstraction
    class Display
    {
    public:
      Display(const Canvas& canvas);
      ~Display();

      void flush();

    private:
      ::Display *m_dpy;
      Window m_win;
      Colormap m_cmap;
      GLXContext m_cx;
      XEvent m_event;
    };

    Progress progressbar_async(std::ostream& ostr,
                               const Dimension::Progress& progress);
  }

  // Print a progress bar of the progress of `progress'
  std::ostream& operator<<(std::ostream& ostr,
                           const Dimension::Progress& progress);
}

#endif // TESTSXX_HPP
