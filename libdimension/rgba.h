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
 * RGBA canvas optimizer interface, used by image optimizers.
 */

#include <stdint.h>

/// RGBA8 optimizer type.
typedef struct dmnsn_rgba8_optimizer {
  dmnsn_canvas_optimizer optimizer;
  uint8_t data[];
} dmnsn_rgba8_optimizer;

/// RGBA16 optimizer type.
typedef struct dmnsn_rgba16_optimizer {
  dmnsn_canvas_optimizer optimizer;
  uint16_t data[];
} dmnsn_rgba16_optimizer;

/// Apply the RGBA8 optimizer to a canvas.
DMNSN_INTERNAL void dmnsn_rgba8_optimize_canvas(dmnsn_pool *pool, dmnsn_canvas *canvas);
/// Apply the RGBA16 optimizer to a canvas.
DMNSN_INTERNAL void dmnsn_rgba16_optimize_canvas(dmnsn_pool *pool, dmnsn_canvas *canvas);

/// RGBA8 optimizer callback.
DMNSN_INTERNAL void dmnsn_rgba8_optimizer_fn(dmnsn_canvas_optimizer *optimizer, const dmnsn_canvas *canvas, size_t x, size_t y);
/// RGBA16 optimizer callback.
DMNSN_INTERNAL void dmnsn_rgba16_optimizer_fn(dmnsn_canvas_optimizer *optimizer, const dmnsn_canvas *canvas, size_t x, size_t y);
