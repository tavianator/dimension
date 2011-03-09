/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
    dmnsn_error("Couldn't start thread.");
  }
}

/** Payload for threads executed by dmnsn_execute_concurrently(). */
typedef struct dmnsn_concurrent_thread_payload {
  dmnsn_concurrent_thread_fn *thread_fn;
  void *arg;
  unsigned int thread, nthreads;
  int ret;
} dmnsn_concurrent_thread_payload;

static void *
dmnsn_concurrent_thread(void *ptr)
{
  dmnsn_concurrent_thread_payload *payload = ptr;
  payload->ret = (*payload->thread_fn)(payload->arg, payload->thread,
                                       payload->nthreads);
  return NULL;
}

int
dmnsn_execute_concurrently(dmnsn_concurrent_thread_fn *thread_fn,
                           void *arg, unsigned int nthreads)
{
  pthread_t threads[nthreads];
  dmnsn_concurrent_thread_payload payloads[nthreads];

  for (unsigned int i = 0; i < nthreads; ++i) {
    payloads[i].thread_fn = thread_fn;
    payloads[i].arg       = arg;
    payloads[i].thread    = i;
    payloads[i].nthreads  = nthreads;
    payloads[i].ret       = -1;
    if (pthread_create(&threads[i], NULL, &dmnsn_concurrent_thread,
                       &payloads[i]) != 0)
    {
      dmnsn_error("Couldn't start worker thread.");
    }
  }

  int ret = 0;
  for (unsigned int i = 0; i < nthreads; ++i) {
    if (pthread_join(threads[i], NULL) == 0) {
      if (payloads[i].ret != 0) {
        ret = payloads[i].ret;
      }
    } else {
      dmnsn_error("Couldn't join worker thread.");
    }
  }

  return ret;
}
