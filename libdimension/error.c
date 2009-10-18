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

#include "dimension.h"
#include <pthread.h>
#include <stdio.h>  /* For fprintf() */
#include <stdlib.h> /* For exit()    */

static void dmnsn_default_fatal_error_fn();
static dmnsn_fatal_error_fn *dmnsn_fatal = &dmnsn_default_fatal_error_fn;

static dmnsn_severity dmnsn_resilience = DMNSN_SEVERITY_MEDIUM;
static pthread_mutex_t dmnsn_resilience_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t dmnsn_fatal_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Called by dmnsn_error macro (don't call directly). */
void
dmnsn_report_error(dmnsn_severity severity, const char *func, unsigned int line,
                   const char *str)
{
  if (severity >= dmnsn_get_resilience()) {
    /* An error more severe than our resilience happened, bail out */
    fprintf(stderr, "Dimension ERROR: %s, line %u: %s\n", func, line, str);
    (*dmnsn_fatal)();
  } else {
    /* A trivial error happened, warn and continue */
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n", func, line, str);
  }
}

/* Return the current resilience, thread-safely. */
dmnsn_severity
dmnsn_get_resilience()
{
  dmnsn_severity resilience;
  if (pthread_mutex_lock(&dmnsn_resilience_mutex) != 0) {
    /* Couldn't lock the mutex, so warn and continue. */
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't lock resilience mutex.");
  }
  resilience = dmnsn_resilience; /* Copy the static variable to a local */
  if (pthread_mutex_unlock(&dmnsn_resilience_mutex) != 0) {
    /* Couldn't unlock the mutex, so warn and continue.  If the mutex was locked
       earlier, the next dmnsn_get/set_resilience is likely to hang. */
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't unlock resilience mutex.");
  }
  return resilience;
}

/* Set the resilience, thread-safely */
void
dmnsn_set_resilience(dmnsn_severity resilience)
{
  if (resilience > DMNSN_SEVERITY_HIGH) {
    /* Tried to set an illegal resilience, bail out */
    fprintf(stderr, "Dimension ERROR: %s, line %u: %s\n", DMNSN_FUNC, __LINE__,
            "Resilience has wrong value.");
    (*dmnsn_fatal)();
  }

  if (pthread_mutex_lock(&dmnsn_resilience_mutex) != 0) {
    /* Couldn't lock the mutex, so warn and continue. */
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't lock resilience mutex.");
  }
  dmnsn_resilience = resilience;
  if (pthread_mutex_unlock(&dmnsn_resilience_mutex) != 0) {
    /* Couldn't unlock the mutex, so warn and continue.  If the mutex was locked
       earlier, the next dmnsn_get/set_resilience is likely to hang. */
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't unlock resilience mutex.");
  }
}

dmnsn_fatal_error_fn *dmnsn_get_fatal_error_fn()
{
  dmnsn_fatal_error_fn *fatal;
  if (pthread_mutex_lock(&dmnsn_fatal_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't lock fatal error handler mutex.");
  }
  fatal = dmnsn_fatal;
  if (pthread_mutex_unlock(&dmnsn_fatal_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't unlock fatal error handler mutex.");
  }
  return fatal;
}

void dmnsn_set_fatal_error_fn(dmnsn_fatal_error_fn *fatal)
{
  if (pthread_mutex_lock(&dmnsn_fatal_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't lock fatal error handler mutex.");
  }
  dmnsn_fatal = fatal;
  if (pthread_mutex_unlock(&dmnsn_fatal_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s, line %u: %s\n",
            DMNSN_FUNC, __LINE__,
            "Couldn't unlock fatal error handler mutex.");
  }
}

static void
dmnsn_default_fatal_error_fn()
{
  exit(EXIT_FAILURE);
}
