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

#include "dimension.h"
#include <math.h> /* For pow(), sqrt() */

/* sRGB white point (CIE D50) */
const dmnsn_CIE_XYZ dmnsn_whitepoint = { .X = 0.9504060171449392,
                                         .Y = 0.9999085943425312,
                                         .Z = 1.089062231497274 };

/* Standard colors */
const dmnsn_color dmnsn_black = {
  .R = 0.0,
  .G = 0.0,
  .B = 0.0,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_white = {
  .R = 1.0,
  .G = 1.0,
  .B = 1.0,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_red = {
  .R = 1.0,
  .G = 0.0,
  .B = 0.0,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_green = {
  .R = 0.0,
  .G = 1.0,
  .B = 0.0,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_blue = {
  .R = 0.0,
  .G = 0.0,
  .B = 1.0,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_magenta = {
  .R = 1.0,
  .G = 0.0,
  .B = 1.0,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_yellow = {
  .R = 1.0,
  .G = 1.0,
  .B = 0.0,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_cyan = {
  .R = 0.0,
  .G = 1.0,
  .B = 1.0,
  .filter = 0.0,
  .trans  = 0.0
};

bool
dmnsn_color_is_black(dmnsn_color color)
{
  return color.R == 0.0 && color.G == 0.0 && color.B == 0.0;
}

/* sRGB's `C' function */
static double dmnsn_sRGB_C(double Clinear) {
  /*
   * If C represents R, G, and B, then the sRGB values are now found as follows:
   *
   *         { 12.92*Clinear,                Clinear <= 0.0031308
   * Csrgb = {                1/2.4
   *         { (1.055)*Clinear      - 0.055, Clinear >  0.0031308
   */
  if (Clinear <= 0.0031308) {
    return 12.92*Clinear;
  } else {
    return 1.055*pow(Clinear, 1.0/2.4) - 0.055;
  }
}

/* Convert a CIE XYZ color to a dmnsn_color */
dmnsn_color
dmnsn_color_from_XYZ(dmnsn_CIE_XYZ XYZ)
{
  dmnsn_color color = {
    .R =  3.2410*XYZ.X - 1.5374*XYZ.Y - 0.4986*XYZ.Z,
    .G = -0.9692*XYZ.X + 1.8760*XYZ.Y + 0.0416*XYZ.Z,
    .B =  0.0556*XYZ.X - 0.2040*XYZ.Y + 1.0570*XYZ.Z,
    .filter = 0.0,
    .trans = 0.0
  };
  color.R = dmnsn_sRGB_C(color.R);
  color.G = dmnsn_sRGB_C(color.G);
  color.B = dmnsn_sRGB_C(color.B);
  return color;
}

/* Convert a CIE xyY color to a dmnsn_color */
dmnsn_color
dmnsn_color_from_xyY(dmnsn_CIE_xyY xyY)
{
  dmnsn_CIE_XYZ ret = {
    .X = xyY.Y*xyY.x/xyY.y,
    .Y = xyY.Y,
    .Z = xyY.Y*(1.0 - xyY.x - xyY.y)/xyY.y,
  };
  return dmnsn_color_from_XYZ(ret);
}

/* Inverse function of CIE L*a*b*'s `f' function, for the reverse conversion */
static double dmnsn_Lab_finv(double t) {
  if (t > 6.0/29.0) {
    return t*t*t;
  } else {
    return 108.0*(t - 16.0/116.0)/841.0;
  }
}

/* Convert a CIE L*a*b* color to a dmnsn_color, relative to the given
   whitepoint. */
dmnsn_color
dmnsn_color_from_Lab(dmnsn_CIE_Lab Lab, dmnsn_CIE_XYZ white)
{
  double fx, fy, fz;
  dmnsn_CIE_XYZ ret;

  fy = (Lab.L + 16.0)/116.0;
  fx = fy + Lab.a/500.0;
  fz = fy - Lab.b/200.0;

  ret.X = white.X*dmnsn_Lab_finv(fx);
  ret.Y = white.Y*dmnsn_Lab_finv(fy);
  ret.Z = white.Z*dmnsn_Lab_finv(fz);
  return dmnsn_color_from_XYZ(ret);
}

/* Convert a CIE L*u*v* color to a dmnsn_color, relative to the given
   whitepoint. */
dmnsn_color
dmnsn_color_from_Luv(dmnsn_CIE_Luv Luv, dmnsn_CIE_XYZ white)
{
  double fy;
  double uprime, unprime, vprime, vnprime;
  dmnsn_CIE_XYZ ret;

  fy = (Luv.L + 16.0)/116.0;

  unprime = 4.0*white.X/(white.X + 15.0*white.Y + 3.0*white.Z);
  uprime  = Luv.u/Luv.L/13.0 + unprime;
  vnprime = 9.0*white.Y/(white.X + 15.0*white.Y + 3.0*white.Z);
  vprime  = Luv.v/Luv.L/13.0 + vnprime;

  ret.Y = white.Y*dmnsn_Lab_finv(fy);
  ret.X = ret.Y*9.0*uprime/vprime/4.0;
  ret.Z = ret.Y*(12.0 - 3*uprime - 20*vprime)/vprime/4.0;
  return dmnsn_color_from_XYZ(ret);
}

/* Convert an sRGB color to a dmnsn_color (actually a no-op) */
dmnsn_color
dmnsn_color_from_sRGB(dmnsn_sRGB sRGB)
{
  dmnsn_color ret = {
    .R = sRGB.R,
    .G = sRGB.G,
    .B = sRGB.B,
    .filter = 0.0,
    .trans = 0.0
  };
  return ret;
}

/* Inverse function of sRGB's `C' function, for the reverse conversion */
static double dmnsn_sRGB_Cinv(double CsRGB) {
  /*
   * If C represents R, G, and B, then the Clinear values are now found as
   * follows:
   *
   *           { Csrgb/12.92,                  Csrgb <= 0.04045
   * Clinear = {                        1/2.4
   *           { ((Csrgb + 0.055)/1.055)     , Csrgb >  0.04045
   */
  if (CsRGB <= 0.040449936) {
    return CsRGB/12.92;
  } else {
    return pow((CsRGB + 0.055)/1.055, 2.4);
  }
}

/* Convert a dmnsn_color to a CIE XYZ color */
dmnsn_CIE_XYZ
dmnsn_XYZ_from_color(dmnsn_color color)
{
  color.R = dmnsn_sRGB_Cinv(color.R);
  color.G = dmnsn_sRGB_Cinv(color.G);
  color.B = dmnsn_sRGB_Cinv(color.B);

  dmnsn_CIE_XYZ ret = {
    .X = 0.4123808838268995*color.R + 0.3575728355732478*color.G
       + 0.1804522977447919*color.B,
    .Y = 0.2126198631048975*color.R + 0.7151387878413206*color.G
       + 0.0721499433963131*color.B,
    .Z = 0.0193434956789248*color.R + 0.1192121694056356*color.G
       + 0.9505065664127130*color.B,
  };
  return ret;
}

/* Convert a dmnsn_color to a CIE xyY color */
dmnsn_CIE_xyY
dmnsn_xyY_from_color(dmnsn_color color)
{
  dmnsn_CIE_XYZ XYZ = dmnsn_XYZ_from_color(color);
  dmnsn_CIE_xyY ret = {
    .x = XYZ.X/(XYZ.X + XYZ.Y + XYZ.Z),
    .y = XYZ.Y/(XYZ.X + XYZ.Y + XYZ.Z),
    .Y = XYZ.Y
  };
  return ret;
}

/* CIE L*a*b*'s `f' function */
static double dmnsn_Lab_f(double t) {
  if (t > 216.0/24389.0) {
    return pow(t, 1.0/3.0);
  } else {
    return 841.0*t/108.0 + 4.0/29.0;
  }
}

/* Convert a dmnsn_color to a CIE L*a*b* color, relative to the given
   whitepoint */
dmnsn_CIE_Lab
dmnsn_Lab_from_color(dmnsn_color color, dmnsn_CIE_XYZ white)
{
  dmnsn_CIE_XYZ XYZ = dmnsn_XYZ_from_color(color);
  dmnsn_CIE_Lab ret;

  ret.L = 116.0*dmnsn_Lab_f(XYZ.Y/white.Y) - 16.0;
  ret.a = 500.0*(dmnsn_Lab_f(XYZ.X/white.X) - dmnsn_Lab_f(XYZ.Y/white.Y));
  ret.b = 200.0*(dmnsn_Lab_f(XYZ.Y/white.Y) - dmnsn_Lab_f(XYZ.Z/white.Z));
  return ret;
}

/* Convert a dmnsn_color to a CIE L*u*v* color, relative to the given
   whitepoint */
dmnsn_CIE_Luv
dmnsn_Luv_from_color(dmnsn_color color, dmnsn_CIE_XYZ white)
{
  dmnsn_CIE_XYZ XYZ = dmnsn_XYZ_from_color(color);
  double uprime, unprime, vprime, vnprime;
  dmnsn_CIE_Luv ret;

  uprime  = 4.0*XYZ.X   / (XYZ.X   + 15.0*XYZ.Y   + 3.0*XYZ.Z);
  unprime = 4.0*white.X / (white.X + 15.0*white.Y + 3.0*white.Z);
  vprime  = 9.0*XYZ.Y   / (XYZ.X   + 15.0*XYZ.Y   + 3.0*XYZ.Z);
  vnprime = 9.0*white.Y / (white.X + 15.0*white.Y + 3.0*white.Z);

  ret.L = 116.0*dmnsn_Lab_f(XYZ.Y/white.Y) - 16.0;
  ret.u = 13.0*ret.L*(uprime - unprime);
  ret.v = 13.0*ret.L*(vprime - vnprime);
  return ret;
}

/* Convert a dmnsn_color to an sRGB color (actually a no-op) */
dmnsn_sRGB
dmnsn_sRGB_from_color(dmnsn_color color)
{
  dmnsn_sRGB sRGB = { .R = color.R, .G = color.G, .B = color.B };
  return sRGB;
}

/* Add two colors */
dmnsn_color
dmnsn_color_add(dmnsn_color c1, dmnsn_color c2)
{
  dmnsn_sRGB sRGB1 = dmnsn_sRGB_from_color(c1);
  dmnsn_sRGB sRGB2 = dmnsn_sRGB_from_color(c2);

  dmnsn_sRGB sRGB = {
    .R = sRGB1.R + sRGB2.R,
    .G = sRGB1.G + sRGB2.G,
    .B = sRGB1.B + sRGB2.B
  };

  dmnsn_color ret = dmnsn_color_from_sRGB(sRGB);

  /* Weighted average of transparencies by intensity */
  dmnsn_CIE_Lab Lab1 = dmnsn_Lab_from_color(ret, dmnsn_whitepoint);
  dmnsn_CIE_Lab Lab2 = dmnsn_Lab_from_color(ret, dmnsn_whitepoint);
  if (Lab1.L + Lab2.L) {
    ret.filter = (Lab1.L*c1.filter + Lab2.L*c2.filter)/(Lab1.L + Lab2.L);
    ret.trans  = (Lab1.L*c1.trans  + Lab2.L*c2.trans )/(Lab1.L + Lab2.L);
  }
  return ret;
}

/* Multiply a color by a scalar */
dmnsn_color
dmnsn_color_mul(double n, dmnsn_color color)
{
  dmnsn_sRGB sRGB = dmnsn_sRGB_from_color(color);
  sRGB.R *= n;
  sRGB.G *= n;
  sRGB.B *= n;

  dmnsn_color ret = dmnsn_color_from_sRGB(sRGB);
  ret.filter = color.filter;
  ret.trans  = color.trans;
  return ret;
}

/* For n in [0, 1] get the color in a gradient between c1 and c2 */
dmnsn_color
dmnsn_color_gradient(dmnsn_color c1, dmnsn_color c2, double n)
{
  dmnsn_sRGB sRGB1 = dmnsn_sRGB_from_color(c1);
  dmnsn_sRGB sRGB2 = dmnsn_sRGB_from_color(c2);

  dmnsn_sRGB sRGB = {
    .R = n*(sRGB2.R - sRGB1.R) + sRGB1.R,
    .G = n*(sRGB2.G - sRGB1.G) + sRGB1.G,
    .B = n*(sRGB2.B - sRGB1.B) + sRGB1.B
  };

  dmnsn_color ret = dmnsn_color_from_sRGB(sRGB);
  ret.filter = n*(c2.filter - c1.filter) + c1.filter;
  ret.trans  = n*(c2.trans  - c1.trans)  + c1.trans;
  return ret;
}

/* Filters `color' through `filter' */
dmnsn_color
dmnsn_color_filter(dmnsn_color color, dmnsn_color filter)
{
  dmnsn_color transmitted = dmnsn_color_mul(filter.trans, color);
  dmnsn_color filtered = dmnsn_color_mul(filter.filter,
                                         dmnsn_color_illuminate(filter, color));

  dmnsn_color ret = dmnsn_color_add(transmitted, filtered);
  ret.filter = filter.filter*color.filter;
  ret.trans  = filter.trans*color.trans;
  return ret;
}

/* Illuminates `color' with `light' */
dmnsn_color
dmnsn_color_illuminate(dmnsn_color light, dmnsn_color color)
{
  /* We use the sRGB primaries */
  dmnsn_sRGB sRGB1 = dmnsn_sRGB_from_color(light);
  dmnsn_sRGB sRGB2 = dmnsn_sRGB_from_color(color);

  dmnsn_sRGB sRGB = {
    .R = sRGB1.R*sRGB2.R,
    .G = sRGB1.G*sRGB2.G,
    .B = sRGB1.B*sRGB2.B
  };

  dmnsn_color ret = dmnsn_color_from_sRGB(sRGB);
  ret.filter = color.filter;
  ret.trans  = color.trans;
  return ret;
}

/* Find the perceptual difference between two colors, using CIE L*a*b* */
double
dmnsn_color_difference(dmnsn_color color1, dmnsn_color color2)
{
  dmnsn_CIE_Lab Lab1, Lab2;

  Lab1 = dmnsn_Lab_from_color(color1, dmnsn_whitepoint);
  Lab2 = dmnsn_Lab_from_color(color2, dmnsn_whitepoint);

  return sqrt((Lab1.L - Lab2.L)*(Lab1.L - Lab2.L)
              + (Lab1.a - Lab2.a)*(Lab1.a - Lab2.a)
              + (Lab1.b - Lab2.b)*(Lab1.b - Lab2.b));
}
