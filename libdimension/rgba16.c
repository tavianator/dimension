/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
dmnsn_rgba16_optimize_canvas(dmnsn_canvas *canvas)
{
  /* Check if we've already optimized this canvas */
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer *, i, canvas->optimizers) {
    if (i->optimizer_fn == dmnsn_rgba16_optimizer_fn) {
      return;
    }
  }

  dmnsn_canvas_optimizer optimizer;
  optimizer.optimizer_fn = dmnsn_rgba16_optimizer_fn;
  optimizer.free_fn = dmnsn_free;
  optimizer.ptr = dmnsn_malloc(4*canvas->width*canvas->height*sizeof(uint16_t));

  dmnsn_canvas_optimize(canvas, &optimizer);
}

/* RGBA16 optimizer callback */
void
dmnsn_rgba16_optimizer_fn(const dmnsn_canvas *canvas, void *ptr,
                          size_t x, size_t y)
{
  uint16_t *pixel = (uint16_t *)ptr + 4*(y*canvas->width + x);
  dmnsn_tcolor tcolor = dmnsn_canvas_get_pixel(canvas, x, y);
  tcolor = dmnsn_tcolor_remove_filter(tcolor);
  tcolor.c = dmnsn_color_to_sRGB(tcolor.c);
  tcolor = dmnsn_tcolor_saturate(tcolor);

  pixel[0] = lround(tcolor.c.R*UINT16_MAX);
  pixel[1] = lround(tcolor.c.G*UINT16_MAX);
  pixel[2] = lround(tcolor.c.B*UINT16_MAX);
  pixel[3] = lround(tcolor.T*UINT16_MAX);
}
