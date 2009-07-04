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

/*
 * A canvas which is rendered to.
 */

#ifndef DIMENSION_CANVAS_H
#define DIMENSION_CANVAS_H

typedef struct {
  /* width, height */
  unsigned int x, y;

  /*
   * Stored in first-quadrant representation (origin is bottom-left).  The pixel
   * at (a,b) is accessible as pixels[b*x + a].
   */
  dmnsn_color *pixels;

  /* An array of dmnsn_canvas_optimizer's */
  dmnsn_array *optimizers;
} dmnsn_canvas;

typedef struct dmnsn_canvas_optimizer dmnsn_canvas_optimizer;

/* Canvas optimizer callback types */
typedef void dmnsn_canvas_optimizer_fn(dmnsn_canvas *canvas,
                                       dmnsn_canvas_optimizer optimizer,
                                       unsigned int x, unsigned int y);
typedef void dmnsn_canvas_optimizer_free_fn(void *ptr);

/* Canvas optimizer */
struct dmnsn_canvas_optimizer {
  dmnsn_canvas_optimizer_fn *optimizer_fn;
  dmnsn_canvas_optimizer_free_fn *free_fn;
  void *ptr;
};

/* Allocate and free a canvas */
dmnsn_canvas *dmnsn_new_canvas(unsigned int x, unsigned int y);
void dmnsn_delete_canvas(dmnsn_canvas *canvas);

/* Set a canvas optimizer */
void dmnsn_optimize_canvas(dmnsn_canvas *canvas,
                           dmnsn_canvas_optimizer optimizer);

/* Pixel accessors */

DMNSN_INLINE dmnsn_color
dmnsn_get_pixel(const dmnsn_canvas *canvas, unsigned int x, unsigned int y)
{
  return canvas->pixels[y*canvas->x + x];
}

void dmnsn_set_pixel(dmnsn_canvas *canvas, unsigned int x, unsigned int y,
                     dmnsn_color color);

#endif /* DIMENSION_CANVAS_H */
