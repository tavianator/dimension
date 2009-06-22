/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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
#include <stdlib.h> /* For malloc */

dmnsn_progress *
dmnsn_new_progress()
{
  dmnsn_progress_element element = { .progress = 0, .total = 1 };
  dmnsn_progress *progress = malloc(sizeof(dmnsn_progress));
  if (progress) {
    progress->elements = dmnsn_new_array(sizeof(dmnsn_progress_element));
    dmnsn_array_push(progress->elements, &element);

    if (pthread_cond_init(&progress->cond, NULL) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                  "Couldn't initialize condition variable.");
    }
    if (pthread_mutex_init(&progress->mutex, NULL) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't initialize mutex.");
    }
  }
  return progress;
}

void
dmnsn_delete_progress(dmnsn_progress *progress)
{
  if (progress) {
    if (pthread_mutex_destroy(&progress->mutex) != 0) {
      dmnsn_error(DMNSN_SEVERITY_LOW, "Leaking mutex.");
    }
    if (pthread_cond_destroy(&progress->cond) != 0) {
      dmnsn_error(DMNSN_SEVERITY_LOW, "Leaking condition variable.");
    }

    dmnsn_delete_array(progress->elements);
    free(progress);
  }
}

int dmnsn_finish_progress(dmnsn_progress *progress)
{
  void *ptr;
  int retval = 1;

  if (progress) {
    if (pthread_cond_broadcast(&progress->cond) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't signal condition variable.");
    }

    if (pthread_join(progress->thread, &ptr) != 0) {
      /* Medium severity because an unjoined thread likely means that the thread
         is incomplete or invalid */
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Joining worker thread failed.");
    } else if (ptr) {
      retval = *(int *)ptr;
      free(ptr);
    }
    dmnsn_delete_progress(progress);
  }

  return retval;
}

double
dmnsn_get_progress(const dmnsn_progress *progress)
{
  dmnsn_progress_element *element;
  double prog = 0.0;
  unsigned int i;

  dmnsn_array_rdlock(progress->elements);
  for (i = 0; i < progress->elements->length; ++i) {
    element = dmnsn_array_at(progress->elements,
                             progress->elements->length - i - 1);
    prog += element->progress;
    prog /= element->total;
  }
  dmnsn_array_unlock(progress->elements);

  return prog;
}

void
dmnsn_new_progress_element(dmnsn_progress *progress, unsigned int total)
{
  dmnsn_progress_element element = { .progress = 0, .total = total };
  dmnsn_array_push(progress->elements, &element);
}

void
dmnsn_increment_progress(dmnsn_progress *progress)
{
  dmnsn_progress_element *element;

  dmnsn_array_wrlock(progress->elements);
    element = dmnsn_array_at(progress->elements, progress->elements->length - 1);
    ++element->progress;

    while (element->progress >= element->total
          && progress->elements->length > 1) {
      dmnsn_array_resize_unlocked(progress->elements,
                                  progress->elements->length - 1);
      element = dmnsn_array_at(progress->elements, progress->elements->length - 1);
      ++element->progress;
    }

    if (pthread_cond_broadcast(&progress->cond) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't signal condition variable.");
    }
  dmnsn_array_unlock(progress->elements);
}

/* Immediately set to 100% completion */
void
dmnsn_progress_done(dmnsn_progress *progress)
{
  dmnsn_progress_element *element;

  dmnsn_array_wrlock(progress->elements);
    dmnsn_array_resize_unlocked(progress->elements, 1);
    element = dmnsn_array_at(progress->elements, progress->elements->length - 1);
    element->progress = element->total;

    if (pthread_cond_broadcast(&progress->cond) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't signal condition variable.");
    }
  dmnsn_array_unlock(progress->elements);
}

/* Wait until dmnsn_get_progress(progress) >= prog */
void
dmnsn_wait_progress(dmnsn_progress *progress, double prog)
{
  if (pthread_mutex_lock(&progress->mutex) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't lock condition mutex.");
  }

  while (dmnsn_get_progress(progress) < prog) {
    if (pthread_cond_wait(&progress->cond, &progress->mutex) != 0) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                  "Couldn't wait on condition variable.");
    }
  }

  if (pthread_mutex_unlock(&progress->mutex) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't unlock condition mutex.");
  }
}
