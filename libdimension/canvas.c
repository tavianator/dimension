/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "dimension-internal.h"
#include <stdlib.h> /* For dmnsn_free() */

/* Allocate a new canvas, of width x and height y */
dmnsn_canvas *
dmnsn_new_canvas(size_t width, size_t height)
{
  dmnsn_canvas *canvas = dmnsn_malloc(sizeof(dmnsn_canvas));
  canvas->width      = width;
  canvas->height     = height;
  canvas->optimizers = dmnsn_new_array(sizeof(dmnsn_canvas_optimizer));
  canvas->pixels     = dmnsn_malloc(sizeof(dmnsn_tcolor)*width*height);
  DMNSN_REFCOUNT_INIT(canvas);
  return canvas;
}

/* Delete a dmnsn_canvas allocated with dmnsn_new_canvas */
void
dmnsn_delete_canvas(dmnsn_canvas *canvas)
{
  if (DMNSN_DECREF(canvas)) {
    /* Free the optimizers */
    DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer *, i, canvas->optimizers) {
      if (i->free_fn) {
        i->free_fn(i->ptr);
      }
    }
    dmnsn_delete_array(canvas->optimizers);

    /* Free the pixels and canvas */
    dmnsn_free(canvas->pixels);
    dmnsn_free(canvas);
  }
}

/* Set a canvas optimizer */
void
dmnsn_canvas_optimize(dmnsn_canvas *canvas,
                      const dmnsn_canvas_optimizer *optimizer)
{
  dmnsn_array_push(canvas->optimizers, optimizer);
}

/* Find an optimizer if it's already installed */
dmnsn_canvas_optimizer *
dmnsn_canvas_find_optimizer(const dmnsn_canvas *canvas,
                            dmnsn_canvas_optimizer_fn *optimizer_fn)
{
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer *, i, canvas->optimizers) {
    if (i->optimizer_fn == optimizer_fn) {
      return i;
    }
  }

  return NULL;
}

/* Set the value of a pixel */
void
dmnsn_canvas_set_pixel(dmnsn_canvas *canvas, size_t x, size_t y,
                       dmnsn_tcolor tcolor)
{
  dmnsn_assert(x < canvas->width && y < canvas->height,
               "Canvas access out of bounds.");
  dmnsn_assert(!dmnsn_tcolor_isnan(tcolor), "Pixel has NaN component.");

  /* Set the pixel */
  canvas->pixels[y*canvas->width + x] = tcolor;

  /* Call the optimizers */
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer *, i, canvas->optimizers) {
    i->optimizer_fn(canvas, i->ptr, x, y);
  }
}

/* Fill a canvas with a solid color */
void
dmnsn_canvas_clear(dmnsn_canvas *canvas, dmnsn_tcolor tcolor)
{
  for (size_t x = 0; x < canvas->width; ++x) {
    for (size_t y = 0; y < canvas->height; ++y) {
      dmnsn_canvas_set_pixel(canvas, x, y, tcolor);
    }
  }
}
