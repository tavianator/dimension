/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

/**
 * @file
 * Stubs for GL functions when compiled with --disable-gl.
 */

#include "dimension.h"
#include <errno.h>

int
dmnsn_gl_optimize_canvas(dmnsn_canvas *canvas)
{
  errno = ENOSYS;
  return -1;
}

int
dmnsn_gl_write_canvas(const dmnsn_canvas *canvas)
{
  errno = ENOSYS;
  return -1;
}

int
dmnsn_gl_read_canvas(dmnsn_canvas *canvas, size_t x0, size_t y0)
{
  errno = ENOSYS;
  return NULL;
}
