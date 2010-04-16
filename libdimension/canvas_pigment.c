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
  v = dmnsn_matrix_vector_mul(pigment->trans_inv, v);

  dmnsn_canvas *canvas = pigment->ptr;

  int x = (fmod(v.x, 1.0) + 1.0)*(canvas->x - 1) + 0.5;
  int y = (fmod(v.y, 1.0) + 1.0)*(canvas->y - 1) + 0.5;
  dmnsn_color c = dmnsn_get_pixel(canvas, x%canvas->x, y%canvas->y);
  return c;
}

static void
dmnsn_canvas_pigment_free_fn(void *ptr)
{
  dmnsn_delete_canvas(ptr);
}
