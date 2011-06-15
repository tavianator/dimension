/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "dimension-impl.h"
#include <pthread.h>
#include <stdio.h>     /* For fprintf() */
#include <stdlib.h>    /* For exit() */

/** Report internal errors in this file. */
#define DMNSN_LOCAL_ERROR(str)                          \
  do {                                                  \
    fprintf(stderr, "Dimension ERROR: %s, %s:%u: %s\n", \
            DMNSN_FUNC, __FILE__, __LINE__, (str));     \
    abort();                                            \
  } while (0)

/** The default fatal error handler. */
static void dmnsn_default_fatal_error_fn(void);

/** The current fatal error handler. */
static dmnsn_fatal_error_fn *dmnsn_fatal = dmnsn_default_fatal_error_fn;
/** Mutex which protects \c dmnsn_fatal. */
static pthread_mutex_t dmnsn_fatal_mutex = PTHREAD_MUTEX_INITIALIZER;

/** The current resilience. */
static bool dmnsn_always_die = false;
/** Mutex which protexts \c dmnsn_always_die. */
static pthread_mutex_t dmnsn_always_die_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Called by dmnsn_error macro (don't call directly) */
void
dmnsn_report_error(bool die, const char *func, const char *file,
                   unsigned int line, const char *str)
{
  if (pthread_mutex_lock(&dmnsn_always_die_mutex) != 0) {
    DMNSN_LOCAL_ERROR("Couldn't lock mutex.");
  }
  bool always_die = dmnsn_always_die;
  if (pthread_mutex_unlock(&dmnsn_always_die_mutex) != 0) {
    DMNSN_LOCAL_ERROR("Couldn't unlock mutex.");
  }

  fprintf(stderr, "Dimension %s: %s, %s:%u: %s\n",
          die ? "ERROR" : "WARNING", func, file, line, str);

  if (die || always_die) {
    /* Prevent infinite recursion if the fatal error function itself calls
       dmnsn_error() */
    static __thread bool thread_exiting = false;

    if (thread_exiting) {
      if (die) {
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
  if (pthread_mutex_lock(&dmnsn_always_die_mutex) != 0) {
    DMNSN_LOCAL_ERROR("Couldn't lock mutex.");
  }
  dmnsn_always_die = always_die;
  if (pthread_mutex_unlock(&dmnsn_always_die_mutex) != 0) {
    DMNSN_LOCAL_ERROR("Couldn't unlock mutex.");
  }
}

dmnsn_fatal_error_fn *
dmnsn_get_fatal_error_fn(void)
{
  dmnsn_fatal_error_fn *fatal;
  if (pthread_mutex_lock(&dmnsn_fatal_mutex) != 0) {
    DMNSN_LOCAL_ERROR("Couldn't lock fatal error handler mutex.");
  }
  fatal = dmnsn_fatal;
  if (pthread_mutex_unlock(&dmnsn_fatal_mutex) != 0) {
    DMNSN_LOCAL_ERROR("Couldn't unlock fatal error handler mutex.");
  }
  return fatal;
}

void
dmnsn_set_fatal_error_fn(dmnsn_fatal_error_fn *fatal)
{
  if (pthread_mutex_lock(&dmnsn_fatal_mutex) != 0) {
    DMNSN_LOCAL_ERROR("Couldn't lock fatal error handler mutex.");
  }
  dmnsn_fatal = fatal;
  if (pthread_mutex_unlock(&dmnsn_fatal_mutex) != 0) {
    DMNSN_LOCAL_ERROR("Couldn't unlock fatal error handler mutex.");
  }
}

static void
dmnsn_default_fatal_error_fn(void)
{
  /* Print a stack trace to standard error */
  dmnsn_backtrace(stderr);

  if (dmnsn_is_main_thread()) {
    exit(EXIT_FAILURE);
  } else {
    pthread_exit(NULL);
  }
}
