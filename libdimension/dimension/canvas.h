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

/**
 * @file
 * A canvas which is rendered to.
 */

#ifndef DIMENSION_CANVAS_H
#define DIMENSION_CANVAS_H

#include <stddef.h>

/** A canvas, or image. */
typedef struct {
  size_t width;  /**< Canvas width */
  size_t height; /**< Canvas height */

  /** An array of <tt>dmnsn_canvas_optimizer</tt>s */
  dmnsn_array *optimizers;

  /**
   * @internal
   * Stored in first-quadrant representation (origin is bottom-left).  The pixel
   * at (a,b) is accessible as pixels[b*width + a].
   */
  dmnsn_color *pixels;
} dmnsn_canvas;

/* Forward-declare dmnsn_canvas_optimizer */
typedef struct dmnsn_canvas_optimizer dmnsn_canvas_optimizer;

/**
 * Canvas optimizer callback type.
 * @param[in] canvas     The canvas that was just updated.
 * @param[in] optimizer  The canvas optimizer itself.
 * @param[in] x          The x-coordinate that was just updated.
 * @param[in] y          The y-coordinate that was just updated.
 */
typedef void dmnsn_canvas_optimizer_fn(const dmnsn_canvas *canvas,
                                       dmnsn_canvas_optimizer optimizer,
                                       size_t x, size_t y);

/** Canvas optimizer */
struct dmnsn_canvas_optimizer {
  dmnsn_canvas_optimizer_fn *optimizer_fn; /**< Optimizer callback */
  dmnsn_free_fn             *free_fn;      /**< Destructor callback */

  void *ptr; /**< Generic pointer. */
};

/**
 * Allocate a new canvas.
 * @param[in] width   The width of the canvas to allocate (in pixels).
 * @param[in] height  The height of the canvas to allocate (in pixels).
 * @return The allocated canvas.
 */
dmnsn_canvas *dmnsn_new_canvas(size_t width, size_t height);

/**
 * Delete a canvas.
 * @param[in,out] canvas  The canvas to delete.
 */
void dmnsn_delete_canvas(dmnsn_canvas *canvas);

/**
 * Set a canvas optimizer
 * @param[in,out] canvas    The canvas to optimize.
 * @param[in]     optimizer The optimizer to use.
 */
void dmnsn_optimize_canvas(dmnsn_canvas *canvas,
                           dmnsn_canvas_optimizer optimizer);

/* Pixel accessors */

/**
 * Get the color of a pixel.
 * @param[in] canvas  The canvas to access.
 * @param[in] x       The x coordinate.
 * @param[in] y       The y coordinate.
 * @return The color of the canvas at (\p x, \p y).
 */
DMNSN_INLINE dmnsn_color
dmnsn_get_pixel(const dmnsn_canvas *canvas, size_t x, size_t y)
{
  dmnsn_assert(x < canvas->width && y < canvas->height,
               "Canvas access out of bounds.");
  return canvas->pixels[y*canvas->width + x];
}

/**
 * Set the color of a pixel.
 * @param[in,out] canvas  The canvas to modify.
 * @param[in]     x       The x coordinate of the pixel.
 * @param[in]     y       The y coordinate of the pixel.
 * @param[in]     color   The color to set the pixel at (\p x, \p y) to.
 */
void dmnsn_set_pixel(dmnsn_canvas *canvas, size_t x, size_t y,
                     dmnsn_color color);

/**
 * Clear a canvas uniformly with a given color.
 * @param[in,out] canvas  The canvas to erase.
 * @param[in]     color   The color to paint it with.
 */
void dmnsn_clear_canvas(dmnsn_canvas *canvas, dmnsn_color color);

#endif /* DIMENSION_CANVAS_H */
