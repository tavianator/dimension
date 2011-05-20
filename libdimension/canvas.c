/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "dimension.h"
#include <stdlib.h> /* For dmnsn_free() */

/* Allocate a new canvas, of width x and height y */
dmnsn_canvas *
dmnsn_new_canvas(size_t width, size_t height)
{
  dmnsn_canvas *canvas = dmnsn_malloc(sizeof(dmnsn_canvas));

  canvas->width      = width;
  canvas->height     = height;
  canvas->optimizers = dmnsn_new_array(sizeof(dmnsn_canvas_optimizer));
  canvas->pixels     = dmnsn_malloc(sizeof(dmnsn_color)*width*height);
  canvas->refcount   = dmnsn_new_refcount();

  return canvas;
}

/* Delete a dmnsn_canvas allocated with dmnsn_new_canvas */
void
dmnsn_delete_canvas(dmnsn_canvas *canvas)
{
  if (canvas && DMNSN_DECREF(canvas)) {
    dmnsn_delete_refcount(canvas->refcount);

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
dmnsn_optimize_canvas(dmnsn_canvas *canvas, dmnsn_canvas_optimizer optimizer)
{
  dmnsn_array_push(canvas->optimizers, &optimizer);
}

/* Set the color of a pixel */
void
dmnsn_set_pixel(dmnsn_canvas *canvas, size_t x, size_t y,
                dmnsn_color color)
{
  dmnsn_assert(x < canvas->width && y < canvas->height,
               "Canvas access out of bounds.");

  /* Set the pixel */
  canvas->pixels[y*canvas->width + x] = color;

  /* Call the optimizers */
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer *, i, canvas->optimizers) {
    i->optimizer_fn(canvas, *i, x, y);
  }
}

/* Fill a canvas with a solid color */
void
dmnsn_clear_canvas(dmnsn_canvas *canvas, dmnsn_color color)
{
  for (size_t x = 0; x < canvas->width; ++x) {
    for (size_t y = 0; y < canvas->height; ++y) {
      dmnsn_set_pixel(canvas, x, y, color);
    }
  }
}
