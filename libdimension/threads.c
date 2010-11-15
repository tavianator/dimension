/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
 * Background threading.
 */

#include "dimension-impl.h"
#include <pthread.h>

/** The payload to pass to the pthread callback. */
typedef struct dmnsn_thread_payload {
  dmnsn_thread_fn *thread_fn;
  void *arg;
  dmnsn_progress *progress;
} dmnsn_thread_payload;

/** Clean up after a thread. */
static void
dmnsn_thread_cleanup(void *arg)
{
  dmnsn_thread_payload *payload = arg;
  dmnsn_progress *progress = payload->progress;
  dmnsn_free(payload);

  dmnsn_done_progress(progress);
}

/** pthread callback -- call the real thread callback. */
static void *
dmnsn_thread(void *arg)
{
  dmnsn_thread_payload *payload = arg;
  int *ret;

  pthread_cleanup_push(&dmnsn_thread_cleanup, payload);
  ret  = dmnsn_malloc(sizeof(int));
  *ret = (*payload->thread_fn)(payload->arg);
  pthread_cleanup_pop(1);
  return ret;
}

void
dmnsn_new_thread(dmnsn_progress *progress, dmnsn_thread_fn *thread_fn,
                 void *arg)
{
  dmnsn_thread_payload *payload = dmnsn_malloc(sizeof(dmnsn_thread_payload));
  payload->thread_fn = thread_fn;
  payload->arg       = arg;
  payload->progress  = progress;

  if (pthread_create(&progress->thread, NULL, &dmnsn_thread, payload) != 0) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't start thread.");
  }
}
