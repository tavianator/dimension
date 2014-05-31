/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Image maps.
 */

#include "dimension.h"

/** Canvas pigment type. */
typedef struct dmnsn_canvas_pigment {
  dmnsn_pigment pigment;
  dmnsn_canvas *canvas;
} dmnsn_canvas_pigment;

/** Canvas pigment color callback. */
static dmnsn_tcolor
dmnsn_canvas_pigment_fn(const dmnsn_pigment *pigment, dmnsn_vector v)
{
  const dmnsn_canvas_pigment *canvas_pigment = (const dmnsn_canvas_pigment *)pigment;
  dmnsn_canvas *canvas = canvas_pigment->canvas;

  size_t x = llround((fmod(v.x, 1.0) + 1.0)*(canvas->width  - 1));
  size_t y = llround((fmod(v.y, 1.0) + 1.0)*(canvas->height - 1));
  return dmnsn_canvas_get_pixel(canvas, x%canvas->width, y%canvas->height);
}

/* Create a canvas color */
dmnsn_pigment *
dmnsn_new_canvas_pigment(dmnsn_pool *pool, dmnsn_canvas *canvas)
{
  dmnsn_canvas_pigment *canvas_pigment = DMNSN_PALLOC(pool, dmnsn_canvas_pigment);
  canvas_pigment->canvas = canvas;

  dmnsn_pigment *pigment = &canvas_pigment->pigment;
  dmnsn_init_pigment(pigment);
  pigment->pigment_fn = dmnsn_canvas_pigment_fn;
  return pigment;
}
