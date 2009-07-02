/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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
#include <GL/gl.h>

/* Write canvas to GL framebuffer.  Returns 0 on success, nonzero on failure */
int
dmnsn_gl_write_canvas(const dmnsn_canvas *canvas)
{
  GLuint *pixels; /* Array of 32-bit ints in RGBA order */
  GLuint *pixel;
  dmnsn_sRGB sRGB;
  dmnsn_color color;
  unsigned int x, y, width, height;

  width = canvas->x;
  height = canvas->y;

  pixels = malloc(4*width*height*sizeof(GLuint));
  if (!pixels) {
    return 1;
  }

  for (y = 0; y < height; ++y) {
    for (x = 0; x < width; ++x) {
      pixel = pixels + 4*(y*width + x);

      color = dmnsn_get_pixel(canvas, x, y);
      sRGB = dmnsn_sRGB_from_color(color);

      /* Saturate R, G, and B to [0, UINT32_MAX] */

      if (sRGB.R <= 0.0) {
        pixel[0] = 0;
      } else if (sRGB.R >= 1.0) {
        pixel[0] = UINT32_MAX;
      } else {
        pixel[0] = sRGB.R*UINT32_MAX;
      }

      if (sRGB.G <= 0.0) {
        pixel[1] = 0;
      } else if (sRGB.G >= 1.0) {
        pixel[1] = UINT32_MAX;
      } else {
        pixel[1] = sRGB.G*UINT32_MAX;
      }

      if (sRGB.B <= 0.0) {
        pixel[2] = 0;
      } else if (sRGB.B >= 1.0) {
        pixel[2] = UINT32_MAX;
      } else {
        pixel[2] = sRGB.B*UINT32_MAX;
      }

      /* color.filter + color.trans is in [0.0, 1.0] by definition */
      pixel[3] = (color.filter + color.trans)*UINT32_MAX;
    }
  }

  glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_INT, pixels);

  free(pixels);
  return 0;
}

/* Read a canvas from a GL framebuffer.  Returns NULL on failure. */
dmnsn_canvas *
dmnsn_gl_read_canvas(unsigned int x0, unsigned int y0,
                     unsigned int width, unsigned int height)
{
  dmnsn_canvas *canvas;
  GLuint *pixels; /* Array of 32-bit ints in RGBA order */
  GLuint *pixel;
  dmnsn_sRGB sRGB;
  dmnsn_color *color;
  unsigned int x, y;

  canvas = dmnsn_new_canvas(width, height);
  if (!canvas) {
    return NULL;
  }

  pixels = malloc(4*width*height*sizeof(GLuint));
  if (!pixels) {
    dmnsn_delete_canvas(canvas);
    return NULL;
  }

  glReadPixels(x0, y0, width, height, GL_RGBA, GL_UNSIGNED_INT, pixels);

  for (y = 0; y < height; ++y) {
    for (x = 0; x < width; ++x) {
      color = dmnsn_pixel_at(canvas, x, y);
      pixel = pixels + 4*(y*width + x);

      sRGB.R = ((double)pixel[0])/UINT32_MAX;
      sRGB.G = ((double)pixel[1])/UINT32_MAX;
      sRGB.B = ((double)pixel[2])/UINT32_MAX;

      *color = dmnsn_color_from_sRGB(sRGB);
      color->filter = ((double)pixel[3])/UINT32_MAX;
    }
  }

  free(pixels);
  return canvas;
}
