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
 * OpenGL import/export.
 */

#include "dimension-internal.h"
#include <GL/gl.h>
#include <stdlib.h>
#include <stdint.h>

// Optimize canvas for GL drawing
int
dmnsn_gl_optimize_canvas(dmnsn_canvas *canvas)
{
  dmnsn_rgba8_optimize_canvas(canvas);
  return 0;
}

// Write canvas to GL framebuffer.  Returns 0 on success, nonzero on failure
int
dmnsn_gl_write_canvas(const dmnsn_canvas *canvas)
{
  size_t width = canvas->width;
  size_t height = canvas->height;

  // Check if we can optimize this
  dmnsn_canvas_optimizer *optimizer =
    dmnsn_canvas_find_optimizer(canvas, dmnsn_rgba8_optimizer_fn);
  if (optimizer) {
    glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, optimizer->ptr);
    return glGetError() == GL_NO_ERROR ? 0 : 1;
  }

  // We couldn't, so transform the canvas to RGB now
  GLubyte *pixels = dmnsn_malloc(4*width*height*sizeof(GLubyte));

  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      GLubyte *pixel = pixels + 4*(y*width + x);

      dmnsn_tcolor tcolor = dmnsn_canvas_get_pixel(canvas, x, y);
      tcolor = dmnsn_tcolor_remove_filter(tcolor);
      tcolor.c = dmnsn_color_to_sRGB(tcolor.c);
      tcolor = dmnsn_tcolor_saturate(tcolor);

      pixel[0] = lround(tcolor.c.R*UINT8_MAX);
      pixel[1] = lround(tcolor.c.G*UINT8_MAX);
      pixel[2] = lround(tcolor.c.B*UINT8_MAX);
      pixel[3] = lround(tcolor.T*UINT8_MAX);
    }
  }

  glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  dmnsn_free(pixels);
  return glGetError() == GL_NO_ERROR ? 0 : 1;
}

// Read a canvas from a GL framebuffer.  Returns NULL on failure.
int
dmnsn_gl_read_canvas(dmnsn_canvas *canvas, size_t x0, size_t y0)
{
  size_t width = canvas->width;
  size_t height = canvas->height;

  // Array of 16-bit ints in RGBA order
  GLushort *pixels = dmnsn_malloc(4*width*height*sizeof(GLushort));
  glReadPixels(x0, y0, width, height, GL_RGBA, GL_UNSIGNED_SHORT, pixels);
  if (glGetError() != GL_NO_ERROR) {
    dmnsn_free(pixels);
    return -1;
  }

  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      GLushort *pixel = pixels + 4*(y*width + x);

      dmnsn_tcolor tcolor = dmnsn_new_tcolor5(
        (double)pixel[0]/UINT16_MAX,
        (double)pixel[1]/UINT16_MAX,
        (double)pixel[2]/UINT16_MAX,
        (double)pixel[3]/UINT16_MAX,
        0.0
      );
      tcolor.c = dmnsn_color_from_sRGB(tcolor.c);
      dmnsn_canvas_set_pixel(canvas, x, y, tcolor);
    }
  }

  dmnsn_free(pixels);
  return 0;
}
