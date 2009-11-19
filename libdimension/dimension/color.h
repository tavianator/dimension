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
 * Types to represent color.
 */

#ifndef DIMENSION_COLOR_H
#define DIMENSION_COLOR_H

/* Internally, we use CIE 1931 XYZ color. */
typedef struct {
  double filter, trans; /* Filter transparancy only lets light of this color
                           through; regular transparancy lets all colors
                           through.  filter + trans should be <= 1.0. */
  double X, Y, Z;
} dmnsn_color;

typedef struct {
  double X, Y, Z; /* X, Y, and Z are tristimulus values, unbounded above zero.
                     Diffuse white is (0.9505, 1, 1.089). */
} dmnsn_CIE_XYZ;

typedef struct {
  double x, y, Y; /* x and y are chromaticity coordinates, and Y is luminance,
                     in the CIE 1931 xyZ color space.  We use an unlimited light
                     model, so x,y in [0, 1] and Y >= 0, with 1 = diffuse
                     white */
} dmnsn_CIE_xyY;

typedef struct {
  double R, G, B; /* CIE 1931 RGB, a linear transformation of CIE XYZ */
} dmnsn_CIE_RGB;

typedef struct {
  double L, a, b; /* L is luminence (100 = diffuse white); a and b are color-
                     opponent dimensions.  This color space is used for color
                     arithmetic. */
} dmnsn_CIE_Lab;

typedef struct {
  double L, u, v; /* L is luminence (100 = diffuse white); u and v are
                     chromaticity coordinates. */
} dmnsn_CIE_Luv;

typedef struct {
  double R, G, B; /* sRGB R, G, and B values */
} dmnsn_sRGB;

/* Standard colors */
extern const dmnsn_color dmnsn_black, dmnsn_white, dmnsn_red, dmnsn_green,
  dmnsn_blue, dmnsn_magenta, dmnsn_yellow, dmnsn_cyan;

/* Standard whitepoint, determined by the conversion of sRGB white to CIE XYZ */
extern const dmnsn_CIE_XYZ dmnsn_whitepoint;

/* Color conversions */

dmnsn_color dmnsn_color_from_XYZ(dmnsn_CIE_XYZ XYZ);
dmnsn_color dmnsn_color_from_xyY(dmnsn_CIE_xyY xyY);
dmnsn_color dmnsn_color_from_RGB(dmnsn_CIE_RGB RGB);
dmnsn_color dmnsn_color_from_Lab(dmnsn_CIE_Lab Lab, dmnsn_CIE_XYZ white);
dmnsn_color dmnsn_color_from_Luv(dmnsn_CIE_Luv Luv, dmnsn_CIE_XYZ white);
dmnsn_color dmnsn_color_from_sRGB(dmnsn_sRGB sRGB);

dmnsn_CIE_XYZ dmnsn_XYZ_from_color(dmnsn_color color);
dmnsn_CIE_xyY dmnsn_xyY_from_color(dmnsn_color color);
dmnsn_CIE_RGB dmnsn_RGB_from_color(dmnsn_color color);
dmnsn_CIE_Lab dmnsn_Lab_from_color(dmnsn_color color, dmnsn_CIE_XYZ white);
dmnsn_CIE_Luv dmnsn_Luv_from_color(dmnsn_color color, dmnsn_CIE_XYZ white);
dmnsn_sRGB    dmnsn_sRGB_from_color(dmnsn_color color);

/* Perceptual color manipulation */
dmnsn_color dmnsn_color_add(dmnsn_color color1, dmnsn_color color2);
dmnsn_color dmnsn_color_mul(double n, dmnsn_color color);
dmnsn_color dmnsn_color_filter(dmnsn_color color, dmnsn_color filter);
dmnsn_color dmnsn_color_illuminate(dmnsn_color light, dmnsn_color color);
double dmnsn_color_difference(dmnsn_color color1, dmnsn_color color2);

#endif /* DIMENSION_COLOR_H */
