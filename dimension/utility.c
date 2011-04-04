/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

bool
dmnsn_strtoi(int *n, const char *nptr, int base)
{
  long ln;
  bool ret = dmnsn_strtol(&ln, nptr, base);
  *n = ln;
  return ret && ln <= INT_MAX && ln >= INT_MIN;
}

bool
dmnsn_strtol(long *n, const char *nptr, int base)
{
  char *endptr;
  errno = 0;
  *n = strtol(nptr, &endptr, base);
  return *endptr == '\0' && endptr != nptr && errno == 0;
}

bool
dmnsn_strtoui(unsigned int *n, const char *nptr, int base)
{
  /* Skip leading whitespace to detect a leading minus sign */
  while (isspace(*nptr)) {
    ++nptr;
  }
  bool neg = false;
  if (nptr[0] == '-') {
    ++nptr;
    if (nptr[0] == '-' || nptr[0] == '+') {
      return false;
    }
    neg = true;
  }

  unsigned long ln;
  bool ret = dmnsn_strtoul(&ln, nptr, base);
  if (neg) {
    *n = -ln;
  } else {
    *n = ln;
  }
  return ret && (ln <= UINT_MAX || nptr[0] == '-');
}

bool
dmnsn_strtoul(unsigned long *n, const char *nptr, int base)
{
  char *endptr;
  errno = 0;
  *n = strtoul(nptr, &endptr, base);
  return *endptr == '\0' && endptr != nptr && errno == 0;
}

bool
dmnsn_strtod(double *n, const char *nptr)
{
  char *endptr;
  errno = 0;
  *n = strtod(nptr, &endptr);
  return *endptr == '\0' && endptr != nptr
    && (errno == 0 || (errno == ERANGE && *n == 0.0));
}

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

  if (location.parent) {
    dmnsn_diagnostic(*location.parent, "-- from here");
  }
}
