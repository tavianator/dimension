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

#include "dimension.h"
#include <pthread.h>
#include <stdlib.h> /* For malloc(), free() */

/* Allocate a new canvas, of width x and height y */
dmnsn_canvas *
dmnsn_new_canvas(unsigned int x, unsigned int y)
{
  /* Allocate the dmnsn_canvas struct */
  dmnsn_canvas *canvas = malloc(sizeof(dmnsn_canvas));

  if (canvas) {
    /* Set the width and height */
    canvas->x = x;
    canvas->y = y;

    /* Allocate room for the optimizers */
    canvas->optimizers = dmnsn_new_array(sizeof(dmnsn_canvas_optimizer));

    /* Allocate the pixels */
    canvas->pixels = malloc(sizeof(dmnsn_color)*x*y);
    if (!canvas->pixels) {
      dmnsn_delete_canvas(canvas);
      return NULL;
    }
  }

  return canvas;
}

/* Delete a dmnsn_canvas allocated with dmnsn_new_canvas */
void
dmnsn_delete_canvas(dmnsn_canvas *canvas)
{
  unsigned int i;
  dmnsn_canvas_optimizer optimizer;

  if (canvas) {
    /* Free the optimizers */
    for (i = 0; i < dmnsn_array_size(canvas->optimizers); ++i) {
      dmnsn_array_get(canvas->optimizers, i, &optimizer);
      if (optimizer.free_fn) {
        (*optimizer.free_fn)(optimizer.ptr);
      }
    }
    dmnsn_delete_array(canvas->optimizers);

    /* Free the pixels and canvas */
    free(canvas->pixels);
    free(canvas);
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
dmnsn_set_pixel(dmnsn_canvas *canvas, unsigned int x, unsigned int y,
                dmnsn_color color)
{
  unsigned int i;
  dmnsn_canvas_optimizer optimizer;

  /* Set the pixel */
  canvas->pixels[y*canvas->x + x] = color;

  /* Call the optimizers */
  for (i = 0; i < dmnsn_array_size(canvas->optimizers); ++i) {
    dmnsn_array_get(canvas->optimizers, i, &optimizer);
    (*optimizer.optimizer_fn)(canvas, optimizer, x, y);
  }
}
