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
 * 16-bit RGBA canvas optimizer.
 */

#include "dimension-internal.h"
#include <stdint.h>

void
dmnsn_rgba8_optimize_canvas(dmnsn_pool *pool, dmnsn_canvas *canvas)
{
  if (dmnsn_canvas_find_optimizer(canvas, dmnsn_rgba8_optimizer_fn)) {
    return;
  }

  size_t ndata = 4*canvas->width*canvas->height;
  dmnsn_rgba8_optimizer *rgba8 = dmnsn_palloc(pool, sizeof(dmnsn_rgba8_optimizer) + ndata*sizeof(uint8_t));

  dmnsn_canvas_optimizer *optimizer = &rgba8->optimizer;
  dmnsn_init_canvas_optimizer(optimizer);
  optimizer->optimizer_fn = dmnsn_rgba8_optimizer_fn;

  dmnsn_canvas_optimize(canvas, optimizer);
}

void
dmnsn_rgba16_optimize_canvas(dmnsn_pool *pool, dmnsn_canvas *canvas)
{
  if (dmnsn_canvas_find_optimizer(canvas, dmnsn_rgba16_optimizer_fn)) {
    return;
  }

  size_t ndata = 4*canvas->width*canvas->height;
  dmnsn_rgba16_optimizer *rgba16 = dmnsn_palloc(pool, sizeof(dmnsn_rgba16_optimizer) + ndata*sizeof(uint16_t));

  dmnsn_canvas_optimizer *optimizer = &rgba16->optimizer;
  dmnsn_init_canvas_optimizer(optimizer);
  optimizer->optimizer_fn = dmnsn_rgba16_optimizer_fn;

  dmnsn_canvas_optimize(canvas, optimizer);
}

void
dmnsn_rgba8_optimizer_fn(dmnsn_canvas_optimizer *optimizer, const dmnsn_canvas *canvas, size_t x, size_t y)
{
  dmnsn_rgba8_optimizer *rgba8 = (dmnsn_rgba8_optimizer *)optimizer;

  uint8_t *pixel = rgba8->data + 4*(y*canvas->width + x);
  dmnsn_tcolor tcolor = dmnsn_canvas_get_pixel(canvas, x, y);
  tcolor = dmnsn_tcolor_remove_filter(tcolor);
  tcolor.c = dmnsn_color_to_sRGB(tcolor.c);
  tcolor = dmnsn_tcolor_saturate(tcolor);

  pixel[0] = lround(tcolor.c.R*UINT8_MAX);
  pixel[1] = lround(tcolor.c.G*UINT8_MAX);
  pixel[2] = lround(tcolor.c.B*UINT8_MAX);
  pixel[3] = lround(tcolor.T*UINT8_MAX);
}

void
dmnsn_rgba16_optimizer_fn(dmnsn_canvas_optimizer *optimizer, const dmnsn_canvas *canvas, size_t x, size_t y)
{
  dmnsn_rgba16_optimizer *rgba16 = (dmnsn_rgba16_optimizer *)optimizer;

  uint16_t *pixel = rgba16->data + 4*(y*canvas->width + x);
  dmnsn_tcolor tcolor = dmnsn_canvas_get_pixel(canvas, x, y);
  tcolor = dmnsn_tcolor_remove_filter(tcolor);
  tcolor.c = dmnsn_color_to_sRGB(tcolor.c);
  tcolor = dmnsn_tcolor_saturate(tcolor);

  pixel[0] = lround(tcolor.c.R*UINT16_MAX);
  pixel[1] = lround(tcolor.c.G*UINT16_MAX);
  pixel[2] = lround(tcolor.c.B*UINT16_MAX);
  pixel[3] = lround(tcolor.T*UINT16_MAX);
}
