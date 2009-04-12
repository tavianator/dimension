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
#include "../libdimension-png/dimension-png.h"

namespace Dimension
{
  PNG_Canvas::~PNG_Canvas()
  {
    if (m_ostr && !m_written) {
      try {
        write();
      } catch (...) {
        dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                    "Writing canvas to PNG failed in PNG_Canvas destructor.");
      }
    }
  }

  void PNG_Canvas::write()
  {
    if (m_written) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                  "Attempt to write canvas to PNG twice.");
      return;
    }

    if (!m_ostr) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                  "Attempt to write canvas to PNG without an output stream.");
      return;
    }      

    FILE* file = fcookie(*m_ostr);
    if (!file) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                  "Couldn't create C++/C IO interface when writing canvas"
                  " to PNG.");
      return;
    }      

    if (dmnsn_png_write_canvas(m_canvas, file)) {
      fclose(file);
      dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                  "Writing canvas to PNG failed.");
      return;
    }

    fclose(file);
    m_written = true;
  }

  void PNG_Canvas::read()
  {
    if (!m_istr) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                  "Attempt to read canvas from PNG without an input stream.");
      return;
    }      

    FILE* file = fcookie(*m_istr);
    if (!file) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                  "Couldn't create C++/C IO interface when reading canvas"
                  " from PNG.");
      return;
    }      

    if (!(m_canvas = dmnsn_png_read_canvas(file))) {
      fclose(file);
      dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                  "Reading canvas from PNG failed.");
      return;
    }

    fclose(file);
  }
}
