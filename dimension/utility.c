/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of Dimension.                                       *
 *                                                                       *
 * Dimension is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU General Public License as published by the *
 * Free Software Foundation; either version 3 of the License, or (at     *
 * your option) any later version.                                       *
 *                                                                       *
 * Dimension is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "utility.h"
#include <stdarg.h>
#include <stdio.h>

void
dmnsn_diagnostic(const char *filename, int line, int col, const char *format,
                 ...)
{
  va_list ap;
  va_start(ap, format);

  if (line >= 0 && col >= 0) {
    fprintf(stderr, "%s:%d:%d: ", filename, line, col);
  } else {
    fprintf(stderr, "%s: ", filename);
  }
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");

  va_end(ap);
}
