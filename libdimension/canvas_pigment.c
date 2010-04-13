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

/* Canvas color pigment callback */
static dmnsn_color dmnsn_canvas_pigment_fn(const dmnsn_pigment *pigment,
                                           dmnsn_vector v);
static void dmnsn_canvas_pigment_free_fn(void *ptr);

/* Create a canvas color */
dmnsn_pigment *
dmnsn_new_canvas_pigment(dmnsn_canvas *canvas)
{
  dmnsn_pigment *pigment = dmnsn_new_pigment();
  pigment->pigment_fn = &dmnsn_canvas_pigment_fn;
  pigment->free_fn    = &dmnsn_canvas_pigment_free_fn;
  pigment->ptr        = canvas;
  return pigment;
}

/* Canvas color callback */
static dmnsn_color
dmnsn_canvas_pigment_fn(const dmnsn_pigment *pigment, dmnsn_vector v)
{
  dmnsn_canvas *canvas = pigment->ptr;

  int x = v.x*(canvas->x - 1) + 0.5;
  int y = v.x*(canvas->x - 1) + 0.5;
  if (x >= 0 && y >= 0 && x < canvas->x && y < canvas->y) {
    return dmnsn_get_pixel(canvas, x, y);
  } else {
    return dmnsn_black;
  }
}

static void
dmnsn_canvas_pigment_free_fn(void *ptr)
{
  dmnsn_delete_canvas(ptr);
}
