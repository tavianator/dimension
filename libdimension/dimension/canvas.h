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

/* 48-bit sRGB color for pixels. */
typedef struct {
  uint16_t r, g, b; /* Red, green, blue */
  uint16_t a, t;    /* Filtered transparancy, normal transparancy */
} dmnsn_pixel;

dmnsn_pixel dmnsn_pixel_from_color(dmnsn_color color);
dmnsn_color dmnsn_color_from_pixel(dmnsn_pixel pixel);

typedef struct {
  unsigned int x, y;

  /*
   * Stored in first-quadrant representation (origin is bottom-left).  The pixel
   * at (a,b) is accessible as pixels[b*x + a].
   */
  dmnsn_pixel *pixels;
} dmnsn_canvas;

dmnsn_canvas *dmnsn_new_canvas(unsigned int x, unsigned int y);
void dmnsn_delete_canvas(dmnsn_canvas *canvas);

#endif /* DIMENSION_CANVAS_H */
