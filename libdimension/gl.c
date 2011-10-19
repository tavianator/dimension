/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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

/* Optimize canvas for GL drawing */
int
dmnsn_gl_optimize_canvas(dmnsn_canvas *canvas)
{
  dmnsn_rgba16_optimize_canvas(canvas);
  return 0;
}

/* Write canvas to GL framebuffer.  Returns 0 on success, nonzero on failure */
int
dmnsn_gl_write_canvas(const dmnsn_canvas *canvas)
{
  GLushort *pixels; /* Array of 16-bit ints in RGBA order */
  GLushort *pixel;
  dmnsn_color color;

  size_t width = canvas->width;
  size_t height = canvas->height;

  /* Check if we can optimize this */
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer *, i, canvas->optimizers) {
    if (i->optimizer_fn == dmnsn_rgba16_optimizer_fn) {
      glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_SHORT, i->ptr);
      return glGetError() == GL_NO_ERROR ? 0 : 1;
    }
  }

  /* We couldn't, so transform the canvas to RGB now */
  pixels = dmnsn_malloc(4*width*height*sizeof(GLushort));

  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      pixel = pixels + 4*(y*width + x);

      color = dmnsn_canvas_get_pixel(canvas, x, y);
      color = dmnsn_remove_filter(color);
      color = dmnsn_color_to_sRGB(color);
      color = dmnsn_color_saturate(color);

      pixel[0] = lround(color.R*UINT16_MAX);
      pixel[1] = lround(color.G*UINT16_MAX);
      pixel[2] = lround(color.B*UINT16_MAX);
      pixel[3] = lround(color.trans*UINT16_MAX);
    }
  }

  glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_SHORT, pixels);

  dmnsn_free(pixels);
  return glGetError() == GL_NO_ERROR ? 0 : 1;
}

/* Read a canvas from a GL framebuffer.  Returns NULL on failure. */
dmnsn_canvas *
dmnsn_gl_read_canvas(size_t x0, size_t y0,
                     size_t width, size_t height)
{
  dmnsn_canvas *canvas = dmnsn_new_canvas(width, height);

  /* Array of 16-bit ints in RGBA order */
  GLushort *pixels = dmnsn_malloc(4*width*height*sizeof(GLushort));

  glReadPixels(x0, y0, width, height, GL_RGBA, GL_UNSIGNED_SHORT, pixels);

  if (glGetError() != GL_NO_ERROR) {
    dmnsn_free(pixels);
    dmnsn_delete_canvas(canvas);
    return NULL;
  }

  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      GLushort *pixel = pixels + 4*(y*width + x);

      dmnsn_color color = dmnsn_new_color5((double)pixel[0]/UINT16_MAX,
                                           (double)pixel[1]/UINT16_MAX,
                                           (double)pixel[2]/UINT16_MAX,
                                           (double)pixel[3]/UINT16_MAX,
                                           0.0);
      color = dmnsn_color_from_sRGB(color);
      dmnsn_canvas_set_pixel(canvas, x, y, color);
    }
  }

  dmnsn_free(pixels);
  return canvas;
}
