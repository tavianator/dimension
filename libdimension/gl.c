/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "dimension.h"
#include <GL/gl.h>
#include <stdlib.h>
#include <stdint.h>

/** GL optimizer callback. */
static void dmnsn_gl_optimizer_fn(const dmnsn_canvas *canvas,
                                  dmnsn_canvas_optimizer optimizer,
                                  size_t x, size_t y);

/* Optimize canvas for GL drawing */
int
dmnsn_gl_optimize_canvas(dmnsn_canvas *canvas)
{
  /* Check if we've already optimized this canvas */
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer *, i, canvas->optimizers) {
    if (i->optimizer_fn == dmnsn_gl_optimizer_fn) {
      return 0;
    }
  }

  dmnsn_canvas_optimizer optimizer;
  optimizer.optimizer_fn = dmnsn_gl_optimizer_fn;
  optimizer.free_fn = dmnsn_free;

  /* Allocate a buffer to hold RGB values */
  optimizer.ptr = dmnsn_malloc(4*canvas->width*canvas->height*sizeof(GLushort));

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
  dmnsn_color color;

  size_t width = canvas->width;
  size_t height = canvas->height;

  /* Check if we can optimize this */
  DMNSN_ARRAY_FOREACH (dmnsn_canvas_optimizer *, i, canvas->optimizers) {
    if (i->optimizer_fn == dmnsn_gl_optimizer_fn) {
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

      /* Saturate R, G, and B to [0, UINT16_MAX] */

      if (color.R <= 0.0) {
        pixel[0] = 0;
      } else if (color.R >= 1.0) {
        pixel[0] = UINT16_MAX;
      } else {
        pixel[0] = color.R*UINT16_MAX;
      }

      if (color.G <= 0.0) {
        pixel[1] = 0;
      } else if (color.G >= 1.0) {
        pixel[1] = UINT16_MAX;
      } else {
        pixel[1] = color.G*UINT16_MAX;
      }

      if (color.B <= 0.0) {
        pixel[2] = 0;
      } else if (color.B >= 1.0) {
        pixel[2] = UINT16_MAX;
      } else {
        pixel[2] = color.B*UINT16_MAX;
      }

      double alpha = dmnsn_color_intensity(color)*color.filter + color.trans;
      if (alpha <= 0.0) {
        pixel[3] = 0;
      } else if (alpha >= 1.0) {
        pixel[3] = UINT16_MAX;
      } else {
        pixel[3] = alpha*UINT16_MAX;
      }
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
                                           0.0,
                                           (double)pixel[3]/UINT16_MAX);
      dmnsn_set_pixel(canvas, x, y, color);
    }
  }

  dmnsn_free(pixels);
  return canvas;
}

/* GL optimizer callback */
static void
dmnsn_gl_optimizer_fn(const dmnsn_canvas *canvas,
                      dmnsn_canvas_optimizer optimizer, size_t x, size_t y)
{
  GLushort *pixel = (GLushort *)optimizer.ptr + 4*(y*canvas->width + x);
  dmnsn_color color = dmnsn_get_pixel(canvas, x, y);

  /* Saturate R, G, and B to [0, UINT16_MAX] */

  if (color.R <= 0.0) {
    pixel[0] = 0;
  } else if (color.R >= 1.0) {
    pixel[0] = UINT16_MAX;
  } else {
    pixel[0] = color.R*UINT16_MAX;
  }

  if (color.G <= 0.0) {
    pixel[1] = 0;
  } else if (color.G >= 1.0) {
    pixel[1] = UINT16_MAX;
  } else {
    pixel[1] = color.G*UINT16_MAX;
  }

  if (color.B <= 0.0) {
    pixel[2] = 0;
  } else if (color.B >= 1.0) {
    pixel[2] = UINT16_MAX;
  } else {
    pixel[2] = color.B*UINT16_MAX;
  }

  double alpha = dmnsn_color_intensity(color)*color.filter + color.trans;
  if (alpha <= 0.0) {
    pixel[3] = 0;
  } else if (alpha >= 1.0) {
    pixel[3] = UINT16_MAX;
  } else {
    pixel[3] = alpha*UINT16_MAX;
  }
}
