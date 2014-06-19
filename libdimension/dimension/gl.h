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
 * OpenGL export/import of canvases.
 */

/**
 * Optimize a canvas for GL drawing
 * @param[in] pool  The memory pool to allocate from.
 * @param[in,out] canvas  The canvas to optimize.
 * @return Whether the canvas was successfully optimized.
 */
int dmnsn_gl_optimize_canvas(dmnsn_pool *pool, dmnsn_canvas *canvas);

/**
 * Write canvas to GL framebuffer.
 * @param[in] canvas  The canvas to draw.
 * @return 0 on success, non-zero on failure.
 */
int dmnsn_gl_write_canvas(const dmnsn_canvas *canvas);

/**
 * Read a canvas from a GL framebuffer.
 * @param[in] canvas  The canvas to write to.
 * @param[in] x0  The \a x screen coordinate to start copying from.
 * @param[in] y0  The \a y screen coordinate to start copying from.
 * @return 0 on success, non-zero on failure.
 */
int dmnsn_gl_read_canvas(dmnsn_canvas *canvas, size_t x0, size_t y0);
