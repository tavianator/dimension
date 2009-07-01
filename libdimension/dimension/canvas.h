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
} dmnsn_canvas;

/* Allocate and free a canvas */
dmnsn_canvas *dmnsn_new_canvas(unsigned int x, unsigned int y);
void dmnsn_delete_canvas(dmnsn_canvas *canvas);

/* Pixel accessors */

DMNSN_INLINE dmnsn_color
dmnsn_get_pixel(const dmnsn_canvas *canvas, unsigned int x, unsigned int y)
{
  return canvas->pixels[y*canvas->x + x];
}

DMNSN_INLINE void
dmnsn_set_pixel(dmnsn_canvas *canvas,
                unsigned int x, unsigned int y, dmnsn_color color)
{
  canvas->pixels[y*canvas->x + x] = color;
}

DMNSN_INLINE dmnsn_color *
dmnsn_pixel_at(dmnsn_canvas *canvas, unsigned int x, unsigned int y)
{
  return canvas->pixels + y*canvas->x + x;
}

#endif /* DIMENSION_CANVAS_H */
