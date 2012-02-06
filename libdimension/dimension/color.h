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
 * Colors.
 */

#include <stdbool.h>

/** A color value. */
typedef struct {
  double R; /**< Red component. */
  double G; /**< Green component. */
  double B; /**< Blue component. */
} dmnsn_color;

/** A standard format string for colors. */
#define DMNSN_COLOR_FORMAT "Color<%g, %g, %g>"
/** The appropriate arguements to printf() a color. */
#define DMNSN_COLOR_PRINTF(c) (c).R, (c).G, (c).B

/** Construct a new color. */
DMNSN_INLINE dmnsn_color
dmnsn_new_color(double R, double G, double B)
{
  dmnsn_color ret = { R, G, B };
  return ret;
}

/** Apply sRGB gamma */
DMNSN_INLINE double
dmnsn_sRGB_gamma(double Clinear)
{
  /*
   * If C represents R, G, and B, then the sRGB values are now found as follows:
   *
   *         { 12.92*Clinear,                Clinear <= 0.0031308
   * Csrgb = {                1/2.4
   *         { (1.055)*Clinear      - 0.055, Clinear >  0.0031308
   */
  if (Clinear == 1.0) {
    return 1.0; /* Map 1.0 to 1.0 instead of 0.9999999999999999 */
  } else if (Clinear > 0.0031308) {
    return 1.055*pow(Clinear, 1.0/2.4) - 0.055;
  } else {
    return 12.92*Clinear;
  }
}

/** Convert to sRGB space. */
DMNSN_INLINE dmnsn_color
dmnsn_color_to_sRGB(dmnsn_color color)
{
  return dmnsn_new_color(
    dmnsn_sRGB_gamma(color.R),
    dmnsn_sRGB_gamma(color.G),
    dmnsn_sRGB_gamma(color.B)
  );
}

/** Remove sRGB gamma */
DMNSN_INLINE double
dmnsn_sRGB_inverse_gamma(double CsRGB)
{
  /*
   * If C represents R, G, and B, then the Clinear values are now found as
   * follows:
   *
   *           { Csrgb/12.92,                  Csrgb <= 0.04045
   * Clinear = {                        1/2.4
   *           { ((Csrgb + 0.055)/1.055)     , Csrgb >  0.04045
   */
  if (CsRGB == 1.0) {
    return 1.0; /* Map 1.0 to 1.0 instead of 0.9999999999999999 */
  } else if (CsRGB <= 0.040449936) {
    return CsRGB/12.92;
  } else {
    return pow((CsRGB + 0.055)/1.055, 2.4);
  }
}

/** Convert from sRGB space. */
DMNSN_INLINE dmnsn_color
dmnsn_color_from_sRGB(dmnsn_color color)
{
  return dmnsn_new_color(
    dmnsn_sRGB_inverse_gamma(color.R),
    dmnsn_sRGB_inverse_gamma(color.G),
    dmnsn_sRGB_inverse_gamma(color.B)
  );
}

/** Greyscale color intensity. */
DMNSN_INLINE double
dmnsn_color_intensity(dmnsn_color color)
{
  return 0.2126*color.R + 0.7152*color.G + 0.0722*color.B;
}

/** Add two colors together. */
DMNSN_INLINE dmnsn_color
dmnsn_color_add(dmnsn_color lhs, dmnsn_color rhs)
{
  return dmnsn_new_color(lhs.R + rhs.R, lhs.G + rhs.G, lhs.B + rhs.B);
}

/** Subtract two colors. */
DMNSN_INLINE dmnsn_color
dmnsn_color_sub(dmnsn_color lhs, dmnsn_color rhs)
{
  return dmnsn_new_color(lhs.R - rhs.R, lhs.G - rhs.G, lhs.B - rhs.B);
}

/** Scale a color's intensity. */
DMNSN_INLINE dmnsn_color
dmnsn_color_mul(double n, dmnsn_color color)
{
  return dmnsn_new_color(n*color.R, n*color.G, n*color.B);
}

/** Return the color at \p n on a gradient from \p c1 at 0 to \p c2 at 1. */
DMNSN_INLINE dmnsn_color
dmnsn_color_gradient(dmnsn_color c1, dmnsn_color c2, double n)
{
  return dmnsn_new_color(
    n*(c2.R - c1.R) + c1.R,
    n*(c2.G - c1.G) + c1.G,
    n*(c2.B - c1.B) + c1.B
  );
}

/** Illuminate \p color with \p light. */
DMNSN_INLINE dmnsn_color
dmnsn_color_illuminate(dmnsn_color light, dmnsn_color color)
{
  return dmnsn_new_color(light.R*color.R, light.G*color.G, light.B*color.B);
}

/** Saturate the color components to [0.0, 1.0]. */
DMNSN_INLINE dmnsn_color
dmnsn_color_saturate(dmnsn_color color)
{
  color.R = dmnsn_min(dmnsn_max(color.R, 0.0), 1.0);
  color.G = dmnsn_min(dmnsn_max(color.G, 0.0), 1.0);
  color.B = dmnsn_min(dmnsn_max(color.B, 0.0), 1.0);
  return color;
}

/** Return whether a color contains any NaN components. */
DMNSN_INLINE bool
dmnsn_color_isnan(dmnsn_color color)
{
  return isnan(color.R) || isnan(color.G) || isnan(color.B);
}

/* Standard colors */

/** Black. */
#define dmnsn_black   dmnsn_new_color(0.0, 0.0, 0.0)
/** White. */
#define dmnsn_white   dmnsn_new_color(1.0, 1.0, 1.0)
/** Red. */
#define dmnsn_red     dmnsn_new_color(1.0, 0.0, 0.0)
/** Green. */
#define dmnsn_green   dmnsn_new_color(0.0, 1.0, 0.0)
/** Blue. */
#define dmnsn_blue    dmnsn_new_color(0.0, 0.0, 1.0)
/** Magenta. */
#define dmnsn_magenta dmnsn_new_color(1.0, 0.0, 1.0)
/** Orange. */
#define dmnsn_orange  dmnsn_new_color(1.0, 0.21404114048223255, 0.0)
/** Yellow. */
#define dmnsn_yellow  dmnsn_new_color(1.0, 1.0, 0.0)
/** Cyan. */
#define dmnsn_cyan    dmnsn_new_color(0.0, 1.0, 1.0)
