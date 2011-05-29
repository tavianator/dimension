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
 * Color-related types and operations.
 */

#include <stdbool.h>

/** A color value. */
typedef struct {
  double R; /**< sRGB red value. */
  double G; /**< sRGB green value. */
  double B; /**< sRGB blue value. */

  double trans;  /**< Translucency. */
  double filter; /**< Degree of filtering. */
} dmnsn_color;

/* Standard colors */
extern const dmnsn_color dmnsn_black;   /**< Black.   */
extern const dmnsn_color dmnsn_white;   /**< White.   */
extern const dmnsn_color dmnsn_clear;   /**< Clear.   */
extern const dmnsn_color dmnsn_red;     /**< Red.     */
extern const dmnsn_color dmnsn_green;   /**< Green.   */
extern const dmnsn_color dmnsn_blue;    /**< Blue.    */
extern const dmnsn_color dmnsn_magenta; /**< Magenta. */
extern const dmnsn_color dmnsn_orange;  /**< Orange.  */
extern const dmnsn_color dmnsn_yellow;  /**< Yellow.  */
extern const dmnsn_color dmnsn_cyan;    /**< Cyan.    */

/* Color construction */

/** Construct a new color. */
DMNSN_INLINE dmnsn_color
dmnsn_new_color(double R, double G, double B)
{
  dmnsn_color ret = { R, G, B, 0.0, 0.0 };
  return ret;
}

/** Construct a new color with transparent components. */
DMNSN_INLINE dmnsn_color
dmnsn_new_color5(double R, double G, double B, double trans, double filter)
{
  dmnsn_color ret = { R, G, B, trans, filter };
  return ret;
}

/** Is this color black? */
DMNSN_INLINE bool
dmnsn_color_is_black(dmnsn_color color)
{
  return fabs(color.R)        < dmnsn_epsilon
         && fabs(color.G)     < dmnsn_epsilon
         && fabs(color.B)     < dmnsn_epsilon
         && fabs(color.trans) < dmnsn_epsilon;
}

/* Perceptual color manipulation */

/** Convert from sRGB space. */
dmnsn_color dmnsn_color_from_sRGB(dmnsn_color color);
/** Convert to sRGB space. */
dmnsn_color dmnsn_color_to_sRGB(dmnsn_color color);
/** Greyscale color intensity. */
double dmnsn_color_intensity(dmnsn_color color);
/** Add two colors together. */
dmnsn_color dmnsn_color_add(dmnsn_color color1, dmnsn_color color2);
/** Multiply a color's intensity by \p n. */
dmnsn_color dmnsn_color_mul(double n, dmnsn_color color);
/** Return the color at \p n on a gradient from \p c1 at 0 to \p c2 at 1. */
dmnsn_color dmnsn_color_gradient(dmnsn_color c1, dmnsn_color c2, double n);
/** Filter \p light through \p filter. */
dmnsn_color dmnsn_filter_light(dmnsn_color light, dmnsn_color filter);
/** Add the background contribution \p filtered to \p filter. */
dmnsn_color dmnsn_apply_translucency(dmnsn_color filtered, dmnsn_color filter);
/** Add the background contribution of \p color to \p filter. */
dmnsn_color dmnsn_apply_filter(dmnsn_color color, dmnsn_color filter);
/** Convert the color into a close equivalent with only transmittance. */
dmnsn_color dmnsn_remove_filter(dmnsn_color color);
/** Illuminate \p color with \p light. */
dmnsn_color dmnsn_color_illuminate(dmnsn_color light, dmnsn_color color);
