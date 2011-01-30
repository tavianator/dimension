/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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
 * Error reporting interface.  Errors are reported at a given severity by the
 * dmnsn_error() macro at a given severity, which prints a warning if it is
 * below the set resilience, or prints an error and exits if it's at or above
 * the set resilience.
 */

#ifndef DIMENSION_ERROR_H
#define DIMENSION_ERROR_H

/** Error severity codes. */
typedef enum dmnsn_severity {
  DMNSN_SEVERITY_LOW,    /**< Only die on low resilience. */
  DMNSN_SEVERITY_MEDIUM, /**< Die on low or medium resilience. */
  DMNSN_SEVERITY_HIGH    /**< Always die. */
} dmnsn_severity;

/**
 * Report an error.
 * @param[in] severity  A @ref dmnsn_severity representing the severity of the
 *                      error.  DMNSN_SEVERITY_HIGH will always terminate the
 *                      running thread.
 * @param[in] str       A string to print explaining the error.
 */
#define dmnsn_error(severity, str)                                             \
  dmnsn_report_error((dmnsn_severity)(severity),                               \
                     DMNSN_FUNC, __FILE__, __LINE__,                           \
                     str)

/**
 * @def dmnsn_assert
 * Make an assertion.
 * @param[in] expr  The expression to assert.
 * @param[in] str   A string to print if the assertion fails.
 */
#ifdef NDEBUG
  #define dmnsn_assert(expr, str) ((void)0)
#else
  #define dmnsn_assert(expr, str)                 \
    do {                                          \
      if (!(expr)) {                              \
        dmnsn_error(DMNSN_SEVERITY_HIGH, (str));  \
      }                                           \
    } while (0)
#endif

/**
 * @internal
 * Called by dmnsn_error(); don't call directly.
 * @param[in] severity  The severity of the error.
 * @param[in] func      The name of the function where the error originated.
 * @param[in] file      The file where the error originated.
 * @param[in] line      The line number where the error originated.
 * @param[in] str       A string describing the error.
 */
void dmnsn_report_error(dmnsn_severity severity,
                        const char *func, const char *file, unsigned int line,
                        const char *str);

/**
 * Get the library resilience, thread-safely.
 * @return The error severity considered fatal.
 */
dmnsn_severity dmnsn_get_resilience(void);

/**
 * Set the library resilience, thread-safely.
 * @param[in] resilience  The new minimum severity that will cause a fatal
 *                        error.
 */
void dmnsn_set_resilience(dmnsn_severity resilience);

/**
 * Fatal error callback type.  This function should never return.
 */
typedef void dmnsn_fatal_error_fn(void);

/**
 * Get the libdimension fatal error handler, thread-safely.  The default fatal
 * error handler terminates the current thread, or the entire program if the
 * current thread is the main thread.
 * @return The current fatal error handler.
 */
dmnsn_fatal_error_fn *dmnsn_get_fatal_error_fn(void);

/**
 * Set the libdimension fatal error handler, thread-safely.
 * @param[in] fatal  The new fatal error handler.  This function must never
 *                   return.
 */
void dmnsn_set_fatal_error_fn(dmnsn_fatal_error_fn *fatal);

#endif /* DIMENSION_ERROR_H */
