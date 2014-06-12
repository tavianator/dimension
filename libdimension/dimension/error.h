/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

#include <stdbool.h>

/**
 * Report a warning.
 * @param[in] str  A string to print explaining the warning.
 */
#define dmnsn_warning(str)                                              \
  dmnsn_report_error(false, DMNSN_FUNC, __FILE__, __LINE__, str)

/**
 * Report an error.
 * @param[in] str  A string to print explaining the error.
 */
#define dmnsn_error(str)                                                \
  do {                                                                  \
    dmnsn_report_error(true, DMNSN_FUNC, __FILE__, __LINE__, str);      \
    DMNSN_UNREACHABLE();                                                \
  } while (0)

/**
 * @def dmnsn_assert
 * Make an assertion.
 * @param[in] expr  The expression to assert.
 * @param[in] str   A string to print if the assertion fails.
 */
#if DMNSN_DEBUG
  #define dmnsn_assert(expr, str)                 \
    do {                                          \
      if (!(expr)) {                              \
        dmnsn_error((str));                       \
      }                                           \
    } while (0)
#else
  #define dmnsn_assert(expr, str) ((void)0)
#endif

/**
 * @def dmnsn_unreachable
 * Express that a line of code is unreachable.
 * @param[in] str  A string to print if the line is reached.
 */
#if DMNSN_DEBUG
  #define dmnsn_unreachable(str) dmnsn_error((str))
#else
  #define dmnsn_unreachable(str) DMNSN_UNREACHABLE()
#endif

/**
 * @internal
 * Called by dmnsn_warning() and dmnsn_error(); don't call directly.
 * @param[in] die   Whether the error is fatal.
 * @param[in] func  The name of the function where the error originated.
 * @param[in] file  The file where the error originated.
 * @param[in] line  The line number where the error originated.
 * @param[in] str   A string describing the error.
 */
void dmnsn_report_error(bool die, const char *func, const char *file,
                        unsigned int line, const char *str);

/**
 * Treat warnings as errors.
 * @param[in] always_die  Whether to die on warnings.
 */
void dmnsn_die_on_warnings(bool always_die);

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
