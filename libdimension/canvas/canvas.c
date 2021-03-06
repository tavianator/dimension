/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Canveses.
 */

#include "dimension/canvas.h"

dmnsn_canvas *
dmnsn_new_canvas(dmnsn_pool *pool, size_t width, size_t height)
{
  dmnsn_canvas *canvas = DMNSN_PALLOC(pool, dmnsn_canvas);
  canvas->width = width;
  canvas->height = height;
  canvas->optimizers = DMNSN_PALLOC_ARRAY(pool, dmnsn_canvas_optimizer *);
  canvas->pixels = dmnsn_palloc(pool, sizeof(dmnsn_tcolor)*width*height);
  return canvas;
}

void
dmnsn_init_canvas_optimizer(dmnsn_canvas_optimizer *optimizer)
{
  optimizer->optimizer_fn = NULL;
}

// Set a canvas optimizer
void
dmnsn_canvas_optimize(dmnsn_canvas *canvas, const dmnsn_canvas_optimizer *optimizer)
{
  dmnsn_array_push(canvas->optimizers, &optimizer);
}

// Find an optimizer if it's already installed
dmnsn_canvas_optimizer *
dmnsn_canvas_find_optimizer(const dmnsn_canvas *canvas, dmnsn_canvas_optimizer_fn *optimizer_fn)
{
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer **, i, canvas->optimizers) {
    if ((*i)->optimizer_fn == optimizer_fn) {
      return *i;
    }
  }

  return NULL;
}

// Set the value of a pixel
void
dmnsn_canvas_set_pixel(dmnsn_canvas *canvas, size_t x, size_t y,
                       dmnsn_tcolor tcolor)
{
  dmnsn_assert(x < canvas->width && y < canvas->height,
               "Canvas access out of bounds.");
  dmnsn_assert(!dmnsn_tcolor_isnan(tcolor), "Pixel has NaN component.");

  // Set the pixel
  canvas->pixels[y*canvas->width + x] = tcolor;

  // Call the optimizers
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer **, i, canvas->optimizers) {
    (*i)->optimizer_fn(*i, canvas, x, y);
  }
}

// Fill a canvas with a solid color
void
dmnsn_canvas_clear(dmnsn_canvas *canvas, dmnsn_tcolor tcolor)
{
  for (size_t x = 0; x < canvas->width; ++x) {
    for (size_t y = 0; y < canvas->height; ++y) {
      dmnsn_canvas_set_pixel(canvas, x, y, tcolor);
    }
  }
}
