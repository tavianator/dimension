/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
dmnsn_diagnostic(dmnsn_parse_location location, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  if (location.first_line >= 0 && location.first_column >= 0) {
    if (location.first_line != location.last_line) {
      fprintf(stderr, "%s:%d-%d: ", location.first_filename,
              location.first_line, location.last_line);
    } else if (location.first_column != location.last_column - 1) {
      fprintf(stderr, "%s:%d:%d-%d: ", location.first_filename,
              location.first_line, location.first_column, location.last_column);
    } else {
      fprintf(stderr, "%s:%d:%d: ", location.first_filename,
              location.first_line, location.first_column);
    }
  } else {
    fprintf(stderr, "%s: ", location.first_filename);
  }
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");

  va_end(ap);
}
