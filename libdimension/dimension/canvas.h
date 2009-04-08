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

/*
 * A canvas which is rendered to.
 */

#ifndef DIMENSION_CANVAS_H
#define DIMENSION_CANVAS_H

#include <pthread.h>

typedef struct {
  unsigned int x, y;

  /*
   * Stored in first-quadrant representation (origin is bottom-left).  The pixel
   * at (a,b) is accessible as pixels[b*x + a].
   */
  dmnsn_color *pixels;

  /* Read-write locks for each pixel */
  pthread_rwlock_t *rwlocks;
} dmnsn_canvas;

dmnsn_canvas *dmnsn_new_canvas(unsigned int x, unsigned int y);
void dmnsn_delete_canvas(dmnsn_canvas *canvas);

/* These handle the rwlocks correctly */
dmnsn_color dmnsn_get_pixel(const dmnsn_canvas *canvas,
                            unsigned int x, unsigned int y);
void dmnsn_set_pixel(dmnsn_canvas *canvas, dmnsn_color color,
                     unsigned int x, unsigned int y);

/* Manual locking */
void dmnsn_rdlock_pixel(const dmnsn_canvas *canvas,
                        unsigned int x, unsigned int y);
void dmnsn_wrlock_pixel(dmnsn_canvas *canvas, unsigned int x, unsigned int y);
void dmnsn_unlock_pixel(const dmnsn_canvas *canvas,
                        unsigned int x, unsigned int y);

#endif /* DIMENSION_CANVAS_H */
