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

#ifndef UTILITY_H
#define UTILITY_H

#include "parse.h" /* For dmnsn_parse_location */
#include <stdbool.h>

/* Wrappers for strtol and strtoul, and some added ones */
bool dmnsn_strtoi(int *n, const char *nptr, int base);
bool dmnsn_strtol(long *n, const char *nptr, int base);
bool dmnsn_strtoui(unsigned int *n, const char *nptr, int base);
bool dmnsn_strtoul(unsigned long *n, const char *nptr, int base);
bool dmnsn_strtod(double *n, const char *nptr);

#if defined(__GNUC__) || defined(__attribute__)
  #define DMNSN_PRINTF_WARN(f, a) __attribute__((format (printf, f, a)))
#else
  #define DMNSN_PRINTF_WARN(f, a)
#endif

/* Print a parsing diagnostic to stderr */
void dmnsn_diagnostic(dmnsn_parse_location location, const char *format, ...)
  DMNSN_PRINTF_WARN(2, 3);

#endif /* UTILITY_H */
