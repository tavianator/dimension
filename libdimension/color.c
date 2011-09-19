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
 * Color handling.
 */

#include "dimension.h"

/* Standard colors */
const dmnsn_color dmnsn_black = {
  .R = 0.0,
  .G = 0.0,
  .B = 0.0,
  .trans  = 0.0,
  .filter = 0.0,
};
const dmnsn_color dmnsn_white = {
  .R = 1.0,
  .G = 1.0,
  .B = 1.0,
  .trans  = 0.0,
  .filter = 0.0,
};
const dmnsn_color dmnsn_clear = {
  .R = 0.0,
  .G = 0.0,
  .B = 0.0,
  .trans  = 1.0,
  .filter = 0.0,
};
const dmnsn_color dmnsn_red = {
  .R = 1.0,
  .G = 0.0,
  .B = 0.0,
  .trans  = 0.0,
  .filter = 0.0,
};
const dmnsn_color dmnsn_green = {
  .R = 0.0,
  .G = 1.0,
  .B = 0.0,
  .trans  = 0.0,
  .filter = 0.0,
};
const dmnsn_color dmnsn_blue = {
  .R = 0.0,
  .G = 0.0,
  .B = 1.0,
  .trans  = 0.0,
  .filter = 0.0,
};
const dmnsn_color dmnsn_magenta = {
  .R = 1.0,
  .G = 0.0,
  .B = 1.0,
  .trans  = 0.0,
  .filter = 0.0,
};
const dmnsn_color dmnsn_orange = {
  .R = 1.0,
  .G = 0.21404114048223255,
  .B = 0.0,
  .trans  = 0.0,
  .filter = 0.0,
};
const dmnsn_color dmnsn_yellow = {
  .R = 1.0,
  .G = 1.0,
  .B = 0.0,
  .trans  = 0.0,
  .filter = 0.0,
};
const dmnsn_color dmnsn_cyan = {
  .R = 0.0,
  .G = 1.0,
  .B = 1.0,
  .filter = 0.0,
  .trans  = 0.0,
};

/** Inverse function of sRGB's `C' function, for the reverse conversion. */
static double
dmnsn_sRGB_C_inv(double CsRGB)
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

/* Convert from sRGB space */
dmnsn_color
dmnsn_color_from_sRGB(dmnsn_color color)
{
  dmnsn_color ret = {
    .R = dmnsn_sRGB_C_inv(color.R),
    .G = dmnsn_sRGB_C_inv(color.G),
    .B = dmnsn_sRGB_C_inv(color.B),
    .trans  = color.trans,
    .filter = color.filter,
  };
  return ret;
}

/** sRGB's `C' function. */
static double
dmnsn_sRGB_C(double Clinear)
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
  } else if (Clinear <= 0.0031308) {
    return 12.92*Clinear;
  } else {
    return 1.055*pow(Clinear, 1.0/2.4) - 0.055;
  }
}

/* Convert to sRGB space */
dmnsn_color
dmnsn_color_to_sRGB(dmnsn_color color)
{
  dmnsn_color ret = {
    .R = dmnsn_sRGB_C(color.R),
    .G = dmnsn_sRGB_C(color.G),
    .B = dmnsn_sRGB_C(color.B),
    .trans  = color.trans,
    .filter = color.filter,
  };
  return ret;
}

/* Greyscale color intensity */
double
dmnsn_color_intensity(dmnsn_color color)
{
  return 0.2126198631048975*color.R + 0.7151387878413206*color.G
         + 0.0721499433963131*color.B;
}

/* Add two colors */
dmnsn_color
dmnsn_color_add(dmnsn_color c1, dmnsn_color c2)
{
  dmnsn_color ret = dmnsn_new_color(c1.R + c2.R, c1.G + c2.G, c1.B + c2.B);

  /* Switch into absolute filter and transmittance space */
  double n1 = dmnsn_color_intensity(c1), n2 = dmnsn_color_intensity(c2);
  double f1 = c1.filter*c1.trans, f2 = c2.filter*c2.trans;
  double t1 = c1.trans - f1, t2 = c2.trans - f2;
  double f = 0.0;
  if (n1 + n2 >= dmnsn_epsilon)
    f = (n1*f1 + n2*f2)/(n1 + n2);
  double t = t1 + t2;

  /* Switch back */
  ret.trans = f + t;
  if (ret.trans >= dmnsn_epsilon)
    ret.filter = f/ret.trans;

  return ret;
}

/* Subtract two colors */
dmnsn_color
dmnsn_color_sub(dmnsn_color c1, dmnsn_color c2)
{
  dmnsn_color ret = dmnsn_new_color(c1.R - c2.R, c1.G - c2.G, c1.B - c2.B);

  /* Switch into absolute filter and transmittance space */
  double n1 = dmnsn_color_intensity(c1), n2 = dmnsn_color_intensity(c2);
  double f1 = c1.filter*c1.trans, f2 = c2.filter*c2.trans;
  double t1 = c1.trans - f1, t2 = c2.trans - f2;
  double f = 0.0;
  if (n1 - n2 >= dmnsn_epsilon)
    f = (n1*f1 - n2*f2)/(n1 - n2);
  double t = t1 - t2;

  /* Switch back */
  ret.trans = f + t;
  if (ret.trans >= dmnsn_epsilon)
    ret.filter = f/ret.trans;

  return ret;
}

/* Multiply a color by a scalar */
dmnsn_color
dmnsn_color_mul(double n, dmnsn_color color)
{
  color.R *= n;
  color.G *= n;
  color.B *= n;
  color.trans *= n;
  return color;
}

/* For n in [0, 1] get the color in a gradient between c1 and c2 */
dmnsn_color
dmnsn_color_gradient(dmnsn_color c1, dmnsn_color c2, double n)
{
  dmnsn_color ret = dmnsn_new_color(
    n*(c2.R - c1.R) + c1.R,
    n*(c2.G - c1.G) + c1.G,
    n*(c2.B - c1.B) + c1.B
  );

  /* Switch into absolute filter and transmittance space */
  double f1 = c1.filter*c1.trans, f2 = c2.filter*c2.trans;
  double t1 = c1.trans - f1, t2 = c2.trans - f2;
  double f = n*(f2 - f1) + f1;
  double t = n*(t2 - t1) + t1;

  /* Switch back */
  ret.trans = f + t;
  ret.filter = 0.0;
  if (ret.trans >= dmnsn_epsilon)
    ret.filter = f/ret.trans;

  return ret;
}

/* Filters `light' through `filter' */
dmnsn_color
dmnsn_filter_light(dmnsn_color light, dmnsn_color filter)
{
  dmnsn_color transmitted = dmnsn_color_mul(
    (1.0 - filter.filter)*filter.trans,
    light
  );
  dmnsn_color filtered = dmnsn_color_mul(
    filter.filter*filter.trans,
    dmnsn_color_illuminate(filter, light)
  );

  dmnsn_color ret = dmnsn_new_color(
    transmitted.R + filtered.R,
    transmitted.G + filtered.G,
    transmitted.B + filtered.B
  );

  /* Switch into absolute filter and transmittance space */
  double lf = light.filter*light.trans, ff = filter.filter*filter.trans;
  double lt = light.trans - lf, ft = filter.trans - ff;
  double f = lf*(dmnsn_color_intensity(filtered) + ft) + lt*ff;
  double t = ft*lt;

  /* Switch back */
  ret.trans = f + t;
  if (ret.trans >= dmnsn_epsilon)
    ret.filter = f/ret.trans;

  return ret;
}

/* Adds the background contribution, `filtered', to `filter' */
dmnsn_color
dmnsn_apply_transparency(dmnsn_color filtered, dmnsn_color filter)
{
  dmnsn_color ret = dmnsn_color_add(
    dmnsn_color_mul(1.0 - filter.trans, filter),
    filtered
  );
  ret.trans  = filtered.trans;
  ret.filter = filtered.filter;
  return ret;
}

/* Adds the background contribution of `color' to `filter' */
dmnsn_color
dmnsn_apply_filter(dmnsn_color color, dmnsn_color filter)
{
  return dmnsn_apply_transparency(dmnsn_filter_light(color, filter), filter);
}

/* Remove the filter channel */
dmnsn_color
dmnsn_remove_filter(dmnsn_color color)
{
  double intensity = dmnsn_color_intensity(color);
  double newtrans = (1.0 - (1.0 - intensity)*color.filter)*color.trans;
  if (1.0 - newtrans >= dmnsn_epsilon)
    color = dmnsn_color_mul((1.0 - color.trans)/(1.0 - newtrans), color);
  color.trans  = newtrans;
  color.filter = 0.0;
  return color;
}

/* Illuminates `color' with `light' */
dmnsn_color
dmnsn_color_illuminate(dmnsn_color light, dmnsn_color color)
{
  return dmnsn_new_color5(light.R*color.R, light.G*color.G, light.B*color.B,
                          color.filter, color.trans);
}
