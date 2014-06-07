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
 * Error handling.
 */

#include "dimension-internal.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/// Report internal errors in this file.
#define DMNSN_LOCAL_ERROR(str)                          \
  do {                                                  \
    fprintf(stderr, "Dimension ERROR: %s, %s:%u: %s\n", \
            DMNSN_FUNC, __FILE__, __LINE__, (str));     \
    abort();                                            \
  } while (0)

/// dmnsn_local_lock_mutex implementation.
static void
dmnsn_local_lock_mutex_impl(pthread_mutex_t *mutex)
{
  if (pthread_mutex_lock(mutex) != 0) {
    DMNSN_LOCAL_ERROR("Couldn't lock mutex.");
  }
}

/// dmnsn_local_unlock_mutex implementation.
static void
dmnsn_local_unlock_mutex_impl(pthread_mutex_t *mutex)
{
  if (pthread_mutex_unlock(mutex) != 0) {
    DMNSN_LOCAL_ERROR("Couldn't lock mutex.");
  }
}

/// Lock a mutex, bailing out without dmnsn_error() on error.
#define dmnsn_local_lock_mutex(mutex)           \
  dmnsn_local_lock_mutex_impl((mutex)); {
/// Unlock a mutex, bailing out without dmnsn_error() on error.
#define dmnsn_local_unlock_mutex(mutex)         \
  dmnsn_local_unlock_mutex_impl((mutex)); }

/// The default fatal error handler.
static void dmnsn_default_fatal_error_fn(void);

/// The current fatal error handler.
static dmnsn_fatal_error_fn *dmnsn_fatal = dmnsn_default_fatal_error_fn;
/// Mutex which protects \c dmnsn_fatal.
static pthread_mutex_t dmnsn_fatal_mutex = PTHREAD_MUTEX_INITIALIZER;

/// The current resilience.
static bool dmnsn_always_die = false;
/// Mutex which protects \c dmnsn_always_die.
static pthread_mutex_t dmnsn_always_die_mutex = PTHREAD_MUTEX_INITIALIZER;

// Called by dmnsn_error macro (don't call directly)
void
dmnsn_report_error(bool die, const char *func, const char *file,
                   unsigned int line, const char *str)
{
  // Save the value of errno
  int err = errno;

  bool always_die;
  dmnsn_local_lock_mutex(&dmnsn_always_die_mutex);
    always_die = dmnsn_always_die;
  dmnsn_local_unlock_mutex(&dmnsn_always_die_mutex);

  // Print the diagnostic string
  fprintf(stderr, "Dimension %s: %s, %s:%u: %s\n",
          die ? "ERROR" : "WARNING", func, file, line, str);

  // Print the value of errno
  if (err != 0) {
    fprintf(stderr, "Last error: %d", err);
#if DMNSN_STRERROR_R
    char errbuf[256];
    if (strerror_r(err, errbuf, 256) == 0) {
      fprintf(stderr, " (%s)", errbuf);
    }
#elif DMNSN_SYS_ERRLIST
    if (err >= 0 && err < sys_nerr) {
      fprintf(stderr, " (%s)", sys_errlist[err]);
    }
#endif
    fprintf(stderr, "\n");
  }

  // Print a stack trace to standard error
  dmnsn_backtrace(stderr);

  // Call the fatal error handler if needed
  if (die || always_die) {
    static __thread bool thread_exiting = false;

    if (thread_exiting) {
      if (die) {
        // Prevent infinite recursion if the fatal error function itself calls
        // dmnsn_error() (not dmnsn_warning()) */
        DMNSN_LOCAL_ERROR("Error raised while in error handler, aborting.");
      }
    } else {
      thread_exiting = true;

      dmnsn_fatal_error_fn *fatal = dmnsn_get_fatal_error_fn();
      fatal();
      DMNSN_LOCAL_ERROR("Fatal error handler didn't exit.");
    }
  }
}

void
dmnsn_die_on_warnings(bool always_die)
{
  dmnsn_local_lock_mutex(&dmnsn_always_die_mutex);
    dmnsn_always_die = always_die;
  dmnsn_local_unlock_mutex(&dmnsn_always_die_mutex);
}

dmnsn_fatal_error_fn *
dmnsn_get_fatal_error_fn(void)
{
  dmnsn_fatal_error_fn *fatal;
  dmnsn_local_lock_mutex(&dmnsn_fatal_mutex);
    fatal = dmnsn_fatal;
  dmnsn_local_unlock_mutex(&dmnsn_fatal_mutex);
  return fatal;
}

void
dmnsn_set_fatal_error_fn(dmnsn_fatal_error_fn *fatal)
{
  dmnsn_local_lock_mutex(&dmnsn_fatal_mutex);
    dmnsn_fatal = fatal;
  dmnsn_local_unlock_mutex(&dmnsn_fatal_mutex);
}

static void
dmnsn_default_fatal_error_fn(void)
{
  if (dmnsn_is_main_thread()) {
    exit(EXIT_FAILURE);
  } else {
    pthread_exit(NULL);
  }
}
