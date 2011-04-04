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
    fprintf(stderr, "Dimension ERROR: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't lock mutex.");
    exit(EXIT_FAILURE);
  }
  bool always_die = dmnsn_always_die;
  if (pthread_mutex_unlock(&dmnsn_always_die_mutex) != 0) {
    fprintf(stderr, "Dimension ERROR: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't unlock mutex.");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Dimension %s: %s, %s:%u: %s\n",
          die ? "ERROR" : "WARNING", func, file, line, str);

  if (die || always_die) {
    /* An error happened, bail out */
    dmnsn_fatal_error_fn *fatal = dmnsn_get_fatal_error_fn();
    fatal();
    exit(EXIT_FAILURE); /* Failsafe in case *dmnsn_fatal doesn't exit */
  }
}

void
dmnsn_die_on_warnings(bool always_die)
{
  if (pthread_mutex_lock(&dmnsn_always_die_mutex) != 0) {
    fprintf(stderr, "Dimension ERROR: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't lock mutex.");
    exit(EXIT_FAILURE);
  }
  dmnsn_always_die = always_die;
  if (pthread_mutex_unlock(&dmnsn_always_die_mutex) != 0) {
    fprintf(stderr, "Dimension ERROR: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't unlock mutex.");
    exit(EXIT_FAILURE);
  }
}

dmnsn_fatal_error_fn *
dmnsn_get_fatal_error_fn(void)
{
  dmnsn_fatal_error_fn *fatal;
  if (pthread_mutex_lock(&dmnsn_fatal_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't lock fatal error handler mutex.");
    exit(EXIT_FAILURE);
  }
  fatal = dmnsn_fatal;
  if (pthread_mutex_unlock(&dmnsn_fatal_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't unlock fatal error handler mutex.");
    exit(EXIT_FAILURE);
  }
  return fatal;
}

void
dmnsn_set_fatal_error_fn(dmnsn_fatal_error_fn *fatal)
{
  if (pthread_mutex_lock(&dmnsn_fatal_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't lock fatal error handler mutex.");
    exit(EXIT_FAILURE);
  }
  dmnsn_fatal = fatal;
  if (pthread_mutex_unlock(&dmnsn_fatal_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't unlock fatal error handler mutex.");
    exit(EXIT_FAILURE);
  }
}

static void
dmnsn_default_fatal_error_fn(void)
{
  /* Prevent infinite recursion if the fatal error function itself calls
     dmnsn_error() */
  static __thread bool thread_exiting = false;

  dmnsn_backtrace(stderr);

  if (thread_exiting) {
    fprintf(stderr, "Dimension ERROR: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Error raised while in error handler, aborting.");
    abort();
  } else if (dmnsn_is_main_thread()) {
    thread_exiting = true;
    exit(EXIT_FAILURE);
  } else {
    thread_exiting = true;
    pthread_exit(NULL);
  }
}
