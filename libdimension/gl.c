/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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
#include <stdlib.h>
#include <stdint.h>

/* GL optimizer callback */
static void dmnsn_gl_optimizer_fn(dmnsn_canvas *canvas,
                                  dmnsn_canvas_optimizer optimizer,
                                  size_t x, size_t y);

/* Optimize canvas for GL drawing */
int
dmnsn_gl_optimize_canvas(dmnsn_canvas *canvas)
{
  /* Check if we've already optimized this canvas */
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer *, i, canvas->optimizers) {
    if (i->optimizer_fn == &dmnsn_gl_optimizer_fn) {
      return 0;
    }
  }

  dmnsn_canvas_optimizer optimizer;
  optimizer.optimizer_fn = &dmnsn_gl_optimizer_fn;
  optimizer.free_fn = &free;

  /* Allocate a buffer to hold RGB values */
  optimizer.ptr = dmnsn_malloc(4*canvas->x*canvas->y*sizeof(GLushort));

  /* Set a new optimizer */
  dmnsn_optimize_canvas(canvas, optimizer);
  return 0;
}

/* Write canvas to GL framebuffer.  Returns 0 on success, nonzero on failure */
int
dmnsn_gl_write_canvas(const dmnsn_canvas *canvas)
{
  GLushort *pixels; /* Array of 16-bit ints in RGBA order */
  GLushort *pixel;
  dmnsn_sRGB sRGB;
  dmnsn_color color;

  size_t width = canvas->x;
  size_t height = canvas->y;

  /* Check if we can optimize this */
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer *, i, canvas->optimizers) {
    if (i->optimizer_fn == &dmnsn_gl_optimizer_fn) {
      glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_SHORT, i->ptr);
      return glGetError() == GL_NO_ERROR ? 0 : 1;
    }
  }

  /* We couldn't, so transform the canvas to RGB now */
  pixels = dmnsn_malloc(4*width*height*sizeof(GLushort));

  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      pixel = pixels + 4*(y*width + x);

      color = dmnsn_get_pixel(canvas, x, y);
      sRGB = dmnsn_sRGB_from_color(color);

      /* Saturate R, G, and B to [0, UINT16_MAX] */

      if (sRGB.R <= 0.0) {
        pixel[0] = 0;
      } else if (sRGB.R >= 1.0) {
        pixel[0] = UINT16_MAX;
      } else {
        pixel[0] = sRGB.R*UINT16_MAX;
      }

      if (sRGB.G <= 0.0) {
        pixel[1] = 0;
      } else if (sRGB.G >= 1.0) {
        pixel[1] = UINT16_MAX;
      } else {
        pixel[1] = sRGB.G*UINT16_MAX;
      }

      if (sRGB.B <= 0.0) {
        pixel[2] = 0;
      } else if (sRGB.B >= 1.0) {
        pixel[2] = UINT16_MAX;
      } else {
        pixel[2] = sRGB.B*UINT16_MAX;
      }

      /* color.filter + color.trans is in [0.0, 1.0] by definition */
      pixel[3] = (color.filter + color.trans)*UINT16_MAX;
    }
  }

  glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_SHORT, pixels);

  free(pixels);
  return glGetError() == GL_NO_ERROR ? 0 : 1;
}

/* Read a canvas from a GL framebuffer.  Returns NULL on failure. */
dmnsn_canvas *
dmnsn_gl_read_canvas(size_t x0, size_t y0,
                     size_t width, size_t height)
{
  dmnsn_canvas *canvas;
  GLushort *pixels; /* Array of 16-bit ints in RGBA order */
  GLushort *pixel;
  dmnsn_sRGB sRGB;
  dmnsn_color color;

  canvas = dmnsn_new_canvas(width, height);
  pixels = dmnsn_malloc(4*width*height*sizeof(GLushort));

  glReadPixels(x0, y0, width, height, GL_RGBA, GL_UNSIGNED_SHORT, pixels);

  if (glGetError() != GL_NO_ERROR) {
    free(pixels);
    dmnsn_delete_canvas(canvas);
    return NULL;
  }

  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      pixel = pixels + 4*(y*width + x);

      sRGB.R = ((double)pixel[0])/UINT16_MAX;
      sRGB.G = ((double)pixel[1])/UINT16_MAX;
      sRGB.B = ((double)pixel[2])/UINT16_MAX;

      color = dmnsn_color_from_sRGB(sRGB);
      color.filter = ((double)pixel[3])/UINT16_MAX;
      dmnsn_set_pixel(canvas, x, y, color);
    }
  }

  free(pixels);
  return canvas;
}

/* GL optimizer callback */
static void
dmnsn_gl_optimizer_fn(dmnsn_canvas *canvas, dmnsn_canvas_optimizer optimizer,
                      size_t x, size_t y)
{
  dmnsn_color color;
  dmnsn_sRGB sRGB;
  GLushort *pixel = (GLushort *)optimizer.ptr + 4*(y*canvas->x + x);

  color = dmnsn_get_pixel(canvas, x, y);
  sRGB = dmnsn_sRGB_from_color(color);

  /* Saturate R, G, and B to [0, UINT16_MAX] */

  if (sRGB.R <= 0.0) {
    pixel[0] = 0;
  } else if (sRGB.R >= 1.0) {
    pixel[0] = UINT16_MAX;
  } else {
    pixel[0] = sRGB.R*UINT16_MAX;
  }

  if (sRGB.G <= 0.0) {
    pixel[1] = 0;
  } else if (sRGB.G >= 1.0) {
    pixel[1] = UINT16_MAX;
  } else {
    pixel[1] = sRGB.G*UINT16_MAX;
  }

  if (sRGB.B <= 0.0) {
    pixel[2] = 0;
  } else if (sRGB.B >= 1.0) {
    pixel[2] = UINT16_MAX;
  } else {
    pixel[2] = sRGB.B*UINT16_MAX;
  }

  /* color.filter + color.trans is in [0.0, 1.0] by definition */
  pixel[3] = (color.filter + color.trans)*UINT16_MAX;
}
