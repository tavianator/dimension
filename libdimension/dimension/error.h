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

/*
 * Error handling.  Errors are reported at a given severity by the dmnsn_error()
 * macro at a given severity, which prints a warning if it is below the set
 * resilience, or prints an error and exits if it's at or above the set
 * resilience.
 */

#ifndef DIMENSION_ERROR_H
#define DIMENSION_ERROR_H

typedef enum {
  DMNSN_SEVERITY_LOW,    /* Only die on low resilience */
  DMNSN_SEVERITY_MEDIUM, /* Die on low or medium resilience */
  DMNSN_SEVERITY_HIGH    /* Always die */
} dmnsn_severity;

#ifdef __GNUC__
  #define DMNSN_FUNC __PRETTY_FUNCTION__
#elif __STDC_VERSION__ >= 199901L
  #define DMNSN_FUNC __func__
#else
  #define DMNSN_FUNC "<unknown function>"
#endif

/* Use this macro to report an error */
#define dmnsn_error(severity, str)                                             \
  dmnsn_report_error((dmnsn_severity)(severity),                               \
                     DMNSN_FUNC, __FILE__, __LINE__,                           \
                     str)

/* Called by dmnsn_error() - don't call directly */
void dmnsn_report_error(dmnsn_severity severity,
                        const char *func, const char *file, unsigned int line,
                        const char *str);

/* Get and set the library resilience, thread-safely */
dmnsn_severity dmnsn_get_resilience();
void dmnsn_set_resilience(dmnsn_severity resilience);

/* Fatal error callback type */
typedef void dmnsn_fatal_error_fn();

/* Get and set libdimension fatal error handling strategy - the default is
   exit(EXIT_FAILURE) */
dmnsn_fatal_error_fn *dmnsn_get_fatal_error_fn();
void dmnsn_set_fatal_error_fn(dmnsn_fatal_error_fn *fatal);

#endif /* DIMENSION_ERROR_H */
