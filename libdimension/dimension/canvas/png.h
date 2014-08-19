/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * PNG import/export of canvases.
 */

#ifndef DMNSN_CANVAS_H
#error "Please include <dimension/canvas.h> instead of this header directly."
#endif

#include <stdio.h>

/**
 * Optimize a canvas for PNG exporting
 * @param[in] pool  The memory pool to allocate from.
 * @param[in,out] canvas  The canvas to optimize.
 * @return Whether the canvas was successfully optimized.
 */
int dmnsn_png_optimize_canvas(dmnsn_pool *pool, dmnsn_canvas *canvas);

/**
 * Write a canvas to a file in PNG format.
 * @param[in] canvas  The canvas to write.
 * @param[in,out] file  The file to write to.
 * @return 0 on success, non-zero on failure.
 */
int dmnsn_png_write_canvas(const dmnsn_canvas *canvas, FILE *file);

/**
 * Write a canvas to a PNG file in the background.
 * @param[in] canvas  The canvas to write.
 * @param[in,out] file  The file to write to.
 * @return A \ref dmnsn_future object, or NULL on failure.
 */
dmnsn_future *dmnsn_png_write_canvas_async(const dmnsn_canvas *canvas,
                                           FILE *file);

/**
 * Read a canvas from a PNG file.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in,out] file  The PNG file to read.
 * @return The new canvas, or NULL on failure.
 */
dmnsn_canvas *dmnsn_png_read_canvas(dmnsn_pool *pool, FILE *file);

/**
 * Read a canvas from a PNG file in the background.
 * @param[out] canvas  The address of a non-allocated canvas object.  The canvas
 *                     object will be allocated and filled with the contents of
 *                     \p file.  Do not read from this object until the
 *                     background task has finished.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in,out] file  The PNG file to read.
 * @return A \ref dmnsn_future object, or NULL on failure.
 */
dmnsn_future *dmnsn_png_read_canvas_async(dmnsn_canvas **canvas, dmnsn_pool *pool, FILE *file);
