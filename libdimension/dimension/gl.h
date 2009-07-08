/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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

/*
 * Draw a canvas to a GL framebuffer with glDrawPixels, or read one with
 * glReadPixels.  Should be fast, so no _async() versions.
 */

#ifndef DIMENSION_GL_H
#define DIMENSION_GL_H

/* Optimize canvas for GL drawing */
int dmnsn_gl_optimize_canvas(dmnsn_canvas *canvas);

/* Write canvas to GL framebuffer.  Returns 0 on success, nonzero on failure. */
int dmnsn_gl_write_canvas(const dmnsn_canvas *canvas);

/* Read a canvas from a GL framebuffer.  Returns NULL on failure. */
dmnsn_canvas *dmnsn_gl_read_canvas(unsigned int x0, unsigned int y0,
                                   unsigned int width, unsigned int height);

#endif /* DIMENSION_GL_H */
