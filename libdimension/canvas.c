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
#include <pthread.h> /* Must be first included header */
#include <stdlib.h>  /* For malloc(), free() */

dmnsn_canvas *
dmnsn_new_canvas(unsigned int x, unsigned int y)
{
  unsigned int i, j, k, l;
  dmnsn_canvas *canvas = malloc(sizeof(dmnsn_canvas));

  if (canvas) {
    canvas->x = x;
    canvas->y = y;
    canvas->pixels = malloc(sizeof(dmnsn_color)*x*y);
    if (!canvas->pixels) {
      free(canvas);
      return NULL;
    }

    canvas->rwlocks = malloc(sizeof(pthread_rwlock_t)*x*y);
    if (!canvas->rwlocks) {
      free(canvas->pixels);
      free(canvas);
      return NULL;
    }

    for (i = 0; i < x; ++i) {
      for (j = 0; j < y; ++j) {
        if (pthread_rwlock_init(&canvas->rwlocks[j*x + i], NULL) != 0) {
          /* pthread_rwlock_init failed.  Destroy the locks we've already made,
             free the canvas, and return NULL.  We leak memory if destruction
             fails (i.e. someone is somehow using an rwlock already). */
          for (l = 0; l < j; ++l) {
            for (k = 0; k < x; ++k) {
              if (pthread_rwlock_destroy(&canvas->rwlocks[l*x + k]) != 0) {
                /* Low severity, because leaked memory won't actually hurt us */
                dmnsn_error(DMNSN_SEVERITY_LOW,
                            "Leaking rwlocks in failed allocation.");
              }
            }
          }

          for (k = 0; k < i; ++k) {
            if (pthread_rwlock_destroy(&canvas->rwlocks[j*x + k]) != 0) {
              dmnsn_error(DMNSN_SEVERITY_LOW,
                          "Leaking rwlocks in failed allocation.");
            }
          }

          free(canvas->rwlocks);
          free(canvas->pixels);
          free(canvas);
          return NULL;
        }
      }
    }
  }

  return canvas;
}

void
dmnsn_delete_canvas(dmnsn_canvas *canvas)
{
  unsigned int i, j;

  if (canvas) {
    for (i = 0; i < canvas->x; ++i) {
      for (j = 0; j < canvas->y; ++j) {
        if (pthread_rwlock_destroy(&canvas->rwlocks[j*canvas->x + i]) != 0) {
              dmnsn_error(DMNSN_SEVERITY_LOW,
                          "Leaking rwlocks in deallocation.");
        }
      }
    }

    free(canvas->rwlocks);
    free(canvas->pixels);
    free(canvas);
  }
}

dmnsn_color
dmnsn_get_pixel(const dmnsn_canvas *canvas, unsigned int x, unsigned int y)
{
  dmnsn_color color;
  dmnsn_rdlock_pixel(canvas, x, y);
  color = canvas->pixels[y*canvas->x + x];
  dmnsn_unlock_pixel(canvas, x, y);
  return color;
}

void
dmnsn_set_pixel(dmnsn_canvas *canvas, dmnsn_color color,
                unsigned int x, unsigned int y)
{
  dmnsn_wrlock_pixel(canvas, x, y);
  canvas->pixels[y*canvas->x + x] = color;
  dmnsn_unlock_pixel(canvas, x, y);
}

void
dmnsn_rdlock_pixel(const dmnsn_canvas *canvas, unsigned int x, unsigned int y)
{
  if (pthread_rwlock_rdlock(&canvas->rwlocks[y*canvas->x + x]) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                "Couldn't acquire read-lock for pixel.");
  }
}

void
dmnsn_wrlock_pixel(dmnsn_canvas *canvas, unsigned int x, unsigned int y)
{
  if (pthread_rwlock_wrlock(&canvas->rwlocks[y*canvas->x + x]) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                "Couldn't acquire write-lock for pixel.");
  }
}

void
dmnsn_unlock_pixel(const dmnsn_canvas *canvas, unsigned int x, unsigned int y)
{
  if (pthread_rwlock_unlock(&canvas->rwlocks[y*canvas->x + x]) != 0) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't unlock pixel.");
  }
}
