/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of Dimension.                                       *
 *                                                                       *
 * Dimension is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU Lesser General Public License as published *
 * by the Free Software Foundation; either version 3 of the License, or  *
 * (at your option) any later version.                                   *
 *                                                                       *
 * Dimension is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
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

static dmnsn_severity dmnsn_resilience = DMNSN_SEVERITY_MEDIUM;
static pthread_mutex_t dmnsn_resilience_mutex = PTHREAD_MUTEX_INITIALIZER;

void
dmnsn_report_error(dmnsn_severity severity, const char *func, const char *str)
{
  if (severity >= dmnsn_get_resilience()) {
    fprintf(stderr, "Dimension ERROR:   %s(): %s\n", func, str);
    exit(1);
  } else {
    fprintf(stderr, "Dimension WARNING: %s(): %s\n", func, str);
  }
}

dmnsn_severity
dmnsn_get_resilience()
{
  dmnsn_severity resilience;
  if (pthread_mutex_lock(&dmnsn_resilience_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s(): %s\n", __func__,
            "Couldn't lock resilience mutex.");
  }
  resilience = dmnsn_resilience;
  if (pthread_mutex_unlock(&dmnsn_resilience_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s(): %s\n", __func__,
            "Couldn't unlock resilience mutex.");
  }
  return resilience;
}

void
dmnsn_set_resilience(dmnsn_severity resilience)
{
  if (resilience > DMNSN_SEVERITY_HIGH) {
    fprintf(stderr, "Dimension ERROR: %s(): %s\n", __func__,
            "Resilience has wrong value.");
    exit(1);
  }

  if (pthread_mutex_lock(&dmnsn_resilience_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s(): %s\n", __func__,
            "Couldn't lock resilience mutex.");
  }
  dmnsn_resilience = resilience;
  if (pthread_mutex_unlock(&dmnsn_resilience_mutex) != 0) {
    fprintf(stderr, "Dimension WARNING: %s(): %s\n", __func__,
            "Couldn't unlock resilience mutex.");
  }
}
