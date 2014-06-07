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
 * RGBA canvas optimizer interface, used by image optimizers.
 */

/// Apply the RGBA8 optimizer to a canvas.
DMNSN_INTERNAL void dmnsn_rgba8_optimize_canvas(dmnsn_canvas *canvas);
/// Apply the RGBA16 optimizer to a canvas.
DMNSN_INTERNAL void dmnsn_rgba16_optimize_canvas(dmnsn_canvas *canvas);

/// RGBA8 optimizer callback.
DMNSN_INTERNAL void dmnsn_rgba8_optimizer_fn(const dmnsn_canvas *canvas,
                                             void *ptr,
                                             size_t x, size_t y);
/// RGBA16 optimizer callback.
DMNSN_INTERNAL void dmnsn_rgba16_optimizer_fn(const dmnsn_canvas *canvas,
                                              void *ptr,
                                              size_t x, size_t y);
