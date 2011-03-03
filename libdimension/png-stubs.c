/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@tavianator.com>          *
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
 * Stubs for PNG functions when compiled with --disable-png.
 */

#include "dimension.h"
#include <errno.h>

int
dmnsn_png_optimize_canvas(dmnsn_canvas *canvas)
{
  errno = ENOSYS;
  return -1;
}

int
dmnsn_png_write_canvas(const dmnsn_canvas *canvas, FILE *file)
{
  errno = ENOSYS;
  return -1;
}

dmnsn_progress *
dmnsn_png_write_canvas_async(const dmnsn_canvas *canvas, FILE *file)
{
  errno = ENOSYS;
  return NULL;
}

dmnsn_canvas *
dmnsn_png_read_canvas(FILE *file)
{
  errno = ENOSYS;
  return NULL;
}

dmnsn_progress *
dmnsn_png_read_canvas_async(dmnsn_canvas **canvas, FILE *file)
{
  errno = ENOSYS;
  return NULL;
}
