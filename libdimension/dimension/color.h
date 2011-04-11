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

#ifndef DIMENSION_COLOR_H
#define DIMENSION_COLOR_H

#include <stdbool.h>

/** A color value. */
typedef struct {
  /** Filtered transparency. */
  double filter;
  /** Unfiltered transparency; <tt>filter + trans</tt> should be <= 1. */
  double trans;

  /* Internally we use sRGB color. */
  double R; /**< @internal sRGB red value. */
  double G; /**< @internal sRGB green value. */
  double B; /**< @internal sRGB blue value. */
} dmnsn_color;

/** sRGB color. */
typedef struct {
  double R; /**< sRGB red value. */
  double G; /**< sRGB green value. */
  double B; /**< sRGB blue value. */
} dmnsn_sRGB;

/** CIE XYZ color. */
typedef struct {
  double X; /**< X component. */
  double Y; /**< Y (luminance) component. */
  double Z; /**< Z component. */
} dmnsn_CIE_XYZ;

/** CIE xyY color. */
typedef struct {
  double x; /**< x chromaticity coordinate (in [0, 1]). */
  double y; /**< y chromaticity coordinate (in [0, 1]). */
  double Y; /**< Luminance, unbounded >= 0; 1 is diffuse white. */
} dmnsn_CIE_xyY;

/** CIE 1976 (L*, a*, b*) color. */
typedef struct {
  double L; /**< Luminance (100 is diffuse white). */
  double a; /**< Red/greed color-opponent value. */
  double b; /**< Yellow/blue color-opponent value. */
} dmnsn_CIE_Lab;

/** CIE 1976 (L*, u*, v*) color. */
typedef struct {
  double L; /**< Luminance (same L* as CIE L*, a*, b*). */
  double u; /**< u* coordinate. */
  double v; /**< v* coordinate. */
} dmnsn_CIE_Luv;

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

/** Standard whitepoint, determined by the conversion of sRGB white to
    CIE XYZ */
extern const dmnsn_CIE_XYZ dmnsn_whitepoint;

/** Is this color black? */
bool dmnsn_color_is_black(dmnsn_color color);

/* Color conversions */

/** Convert an sRGB color to a Dimension color. */
dmnsn_color dmnsn_color_from_sRGB(dmnsn_sRGB sRGB);
/** Convert a CIE XYZ color to a Dimension color. */
dmnsn_color dmnsn_color_from_XYZ(dmnsn_CIE_XYZ XYZ);
/** Convert a CIE xyY color to a Dimension color. */
dmnsn_color dmnsn_color_from_xyY(dmnsn_CIE_xyY xyY);
/** Convert a CIE L*, a*, b* color to a Dimension color. */
dmnsn_color dmnsn_color_from_Lab(dmnsn_CIE_Lab Lab, dmnsn_CIE_XYZ white);
/** Convert a CIE L*, u*, v* color to a Dimension color. */
dmnsn_color dmnsn_color_from_Luv(dmnsn_CIE_Luv Luv, dmnsn_CIE_XYZ white);

/** Convert a Dimension color to sRGB. */
dmnsn_sRGB dmnsn_sRGB_from_color(dmnsn_color color);
/** Convert a Dimension color to CIE XYZ. */
dmnsn_CIE_XYZ dmnsn_XYZ_from_color(dmnsn_color color);
/** Convert a Dimension color to CIE xyY. */
dmnsn_CIE_xyY dmnsn_xyY_from_color(dmnsn_color color);
/** Convert a Dimension color to CIE L*, a*, b*. */
dmnsn_CIE_Lab dmnsn_Lab_from_color(dmnsn_color color, dmnsn_CIE_XYZ white);
/** Convert a Dimension color to CIE L*, u*, v*. */
dmnsn_CIE_Luv dmnsn_Luv_from_color(dmnsn_color color, dmnsn_CIE_XYZ white);

/* Perceptual color manipulation */

/** Add two colors together. */
dmnsn_color dmnsn_color_add(dmnsn_color color1, dmnsn_color color2);
/** Multiply a color's intensity by \p n. */
dmnsn_color dmnsn_color_mul(double n, dmnsn_color color);
/** Return the color at \p n on a gradient from \p c1 at 0 to \p c2 at 1. */
dmnsn_color dmnsn_color_gradient(dmnsn_color c1, dmnsn_color c2, double n);
/** Filter \p color through \p filter. */
dmnsn_color dmnsn_color_filter(dmnsn_color color, dmnsn_color filter);
/** Illuminate \p color with \p light. */
dmnsn_color dmnsn_color_illuminate(dmnsn_color light, dmnsn_color color);

/** Return the perceptual difference between two colors. */
double dmnsn_color_difference(dmnsn_color color1, dmnsn_color color2);

#endif /* DIMENSION_COLOR_H */
