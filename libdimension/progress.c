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
  dmnsn_progress *progress = malloc(sizeof(dmnsn_progress));
  if (progress) {
    progress->elements = dmnsn_new_array(sizeof(dmnsn_progress_element));
  }
  return progress;
}

void
dmnsn_delete_progress(dmnsn_progress *progress)
{
  if (progress) {
    dmnsn_delete_array(progress->elements);
    free(progress);
  }
}

int dmnsn_finish_progress(dmnsn_progress *progress)
{
  void *ptr;
  int retval = 1;

  if (progress) {
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
dmnsn_get_progress(const dmnsn_progress* progress)
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
dmnsn_new_progress_element(dmnsn_progress* progress, unsigned int total)
{
  dmnsn_progress_element element = { .progress = 0, .total = total };
  dmnsn_array_push(progress->elements, &element);
}

void
dmnsn_increment_progress(dmnsn_progress* progress)
{
  dmnsn_progress_element *element;

  dmnsn_array_wrlock(progress->elements);
  element = dmnsn_array_at(progress->elements, progress->elements->length - 1);
  ++element->progress;
  if (element->progress >= element->total) {
    dmnsn_array_resize_unlocked(progress->elements,
                                progress->elements->length - 1);
  }
  dmnsn_array_unlock(progress->elements);
}
