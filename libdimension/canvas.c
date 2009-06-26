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

    /* Allocate the pixels */
    canvas->pixels = malloc(sizeof(dmnsn_color)*x*y);
    if (!canvas->pixels) {
      free(canvas);
      return NULL;
    }
  }

  return canvas;
}

/* Delete a dmnsn_canvas allocated with dmnsn_new_canvas */
void
dmnsn_delete_canvas(dmnsn_canvas *canvas)
{
  if (canvas) {
    /* Free the pixels and canvas */
    free(canvas->pixels);
    free(canvas);
  }
}

/* Get a pixel at (x,y) */
dmnsn_color
dmnsn_get_pixel(const dmnsn_canvas *canvas, unsigned int x, unsigned int y)
{
  return canvas->pixels[y*canvas->x + x];
}

/* Set a pixel at (x,y) */
void
dmnsn_set_pixel(dmnsn_canvas *canvas,
                unsigned int x, unsigned int y, dmnsn_color color)
{
  canvas->pixels[y*canvas->x + x] = color;
}

/* Point to the pixel at (x,y) */
dmnsn_color *
dmnsn_pixel_at(dmnsn_canvas *canvas, unsigned int x, unsigned int y)
{
  return canvas->pixels + y*canvas->x + x;
}
