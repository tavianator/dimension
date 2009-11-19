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
  .X = 0.0,
  .Y = 0.0,
  .Z = 0.0,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_white = {
  .X = 0.9504060171449392,
  .Y = 0.9999085943425312,
  .Z = 1.089062231497274,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_red = {
  .X = 0.4123808838268995,
  .Y = 0.2126198631048975,
  .Z = 0.0193434956789248,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_green = {
  .X = 0.3575728355732478,
  .Y = 0.7151387878413206,
  .Z = 0.1192121694056356,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_blue = {
  .X = 0.1804522977447919,
  .Y = 0.0721499433963131,
  .Z = 0.950506566412713,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_magenta = {
  .X = 0.5928331815716914,
  .Y = 0.2847698065012106,
  .Z = 0.9698500620916378,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_yellow = {
  .X = 0.7699537194001473,
  .Y = 0.9277586509462181,
  .Z = 0.1385556650845604,
  .filter = 0.0,
  .trans  = 0.0
};
const dmnsn_color dmnsn_cyan = {
  .X = 0.5380251333180397,
  .Y = 0.7872887312376337,
  .Z = 1.069718735818349,
  .filter = 0.0,
  .trans  = 0.0
};

/* Convert a CIE XYZ color to a dmnsn_color (actually a no-op) */
dmnsn_color
dmnsn_color_from_XYZ(dmnsn_CIE_XYZ XYZ)
{
  dmnsn_color ret = { .X = XYZ.X, .Y = XYZ.Y, .Z = XYZ.Z,
                      .filter = 0.0, .trans = 0.0 };
  return ret;
}

/* Convert a CIE xyY color to a dmnsn_color */
dmnsn_color
dmnsn_color_from_xyY(dmnsn_CIE_xyY xyY)
{
  dmnsn_color ret = {
    .X = xyY.Y*xyY.x/xyY.y,
    .Y = xyY.Y,
    .Z = xyY.Y*(1.0 - xyY.x - xyY.y)/xyY.y,
    .filter = 0.0,
    .trans = 0.0
  };
  return ret;
}

dmnsn_color
dmnsn_color_from_RGB(dmnsn_CIE_RGB RGB)
{
  dmnsn_color ret = {
    .X = 0.49*RGB.R    + 0.31*RGB.G    + 0.20*RGB.B,
    .Y = 0.17697*RGB.R + 0.81240*RGB.G + 0.01063*RGB.B,
    .Z = 0.00*RGB.R    + 0.01*RGB.G    + 0.99*RGB.B,
    .filter = 0.0,
    .trans = 0.0
  };
  return ret;
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
  dmnsn_color ret;

  fy = (Lab.L + 16.0)/116.0;
  fx = fy + Lab.a/500.0;
  fz = fy - Lab.b/200.0;

  ret.X = white.X*dmnsn_Lab_finv(fx);
  ret.Y = white.Y*dmnsn_Lab_finv(fy);
  ret.Z = white.Z*dmnsn_Lab_finv(fz);
  ret.filter = 0.0;
  ret.trans  = 0.0;

  return ret;
}

/* Convert a CIE L*u*v* color to a dmnsn_color, relative to the given
   whitepoint. */
dmnsn_color
dmnsn_color_from_Luv(dmnsn_CIE_Luv Luv, dmnsn_CIE_XYZ white)
{
  double fy;
  double uprime, unprime, vprime, vnprime;
  dmnsn_color ret;

  fy = (Luv.L + 16.0)/116.0;

  unprime = 4.0*white.X/(white.X + 15.0*white.Y + 3.0*white.Z);
  uprime  = Luv.u/Luv.L/13.0 + unprime;
  vnprime = 9.0*white.Y/(white.X + 15.0*white.Y + 3.0*white.Z);
  vprime  = Luv.v/Luv.L/13.0 + vnprime;

  ret.Y = white.Y*dmnsn_Lab_finv(fy);
  ret.X = ret.Y*9.0*uprime/vprime/4.0;
  ret.Z = ret.Y*(12.0 - 3*uprime - 20*vprime)/vprime/4.0;
  ret.filter = 0.0;
  ret.trans  = 0.0;

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

/* Convert an sRGB value to a dmnsn_color */
dmnsn_color
dmnsn_color_from_sRGB(dmnsn_sRGB sRGB)
{
  double Rlinear, Glinear, Blinear; /* Linear RGB values - no gamma */
  dmnsn_color ret;

  Rlinear = dmnsn_sRGB_Cinv(sRGB.R);
  Glinear = dmnsn_sRGB_Cinv(sRGB.G);
  Blinear = dmnsn_sRGB_Cinv(sRGB.B);

  /*
   * Now, the linear conversion.  Expressed as matrix multiplication, it looks
   * like this:
   *
   *   [X]   [0.4124 0.3576 0.1805] [Rlinear]
   *   [Y] = [0.2126 0.7152 0.0722]*[Glinear]
   *   [X]   [0.0193 0.1192 0.9505] [Blinear]
   */
  ret.X = 0.4123808838268995*Rlinear + 0.3575728355732478*Glinear
    + 0.1804522977447919*Blinear;
  ret.Y = 0.2126198631048975*Rlinear + 0.7151387878413206*Glinear
    + 0.0721499433963131*Blinear;
  ret.Z = 0.0193434956789248*Rlinear + 0.1192121694056356*Glinear
    + 0.9505065664127130*Blinear;
  ret.filter = 0.0;
  ret.trans  = 0.0;

  return ret;
}

/* Convert a dmnsn_color to a CIE XYZ color (actually a no-op) */
dmnsn_CIE_XYZ
dmnsn_XYZ_from_color(dmnsn_color color)
{
  dmnsn_CIE_XYZ ret = { .X = color.X, .Y = color.Y, .Z = color.Z };
  return ret;
}

/* Convert a dmnsn_color to a CIE xyY color */
dmnsn_CIE_xyY
dmnsn_xyY_from_color(dmnsn_color color)
{
  dmnsn_CIE_xyY ret = {
    .x = color.X/(color.X + color.Y + color.Z),
    .y = color.Y/(color.X + color.Y + color.Z),
    .Y = color.Y
  };
  return ret;
}

dmnsn_CIE_RGB
dmnsn_RGB_from_color(dmnsn_color color)
{
  dmnsn_CIE_RGB ret = {
    .R = 2.36461384653836548*color.X + -0.89654057073966797*color.Y
      + -0.46807327579869740*color.Z,
    .G = -0.51516620844788796*color.X + 1.42640810385638872*color.Y
      + 0.08875810459149917*color.Z,
    .B = 0.00520369907523119*color.X + -0.01440816266521605*color.Y
      + 1.00920446358998483*color.Z
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
  dmnsn_CIE_Lab ret;

  ret.L = 116.0*dmnsn_Lab_f(color.Y/white.Y) - 16.0;
  ret.a = 500.0*(dmnsn_Lab_f(color.X/white.X) - dmnsn_Lab_f(color.Y/white.Y));
  ret.b = 200.0*(dmnsn_Lab_f(color.Y/white.Y) - dmnsn_Lab_f(color.Z/white.Z));

  return ret;
}

/* Convert a dmnsn_color to a CIE L*u*v* color, relative to the given
   whitepoint */
dmnsn_CIE_Luv
dmnsn_Luv_from_color(dmnsn_color color, dmnsn_CIE_XYZ white)
{
  double uprime, unprime, vprime, vnprime;
  dmnsn_CIE_Luv ret;

  uprime  = 4.0*color.X/(color.X + 15.0*color.Y + 3.0*color.Z);
  unprime = 4.0*white.X/(white.X + 15.0*white.Y + 3.0*white.Z);
  vprime  = 9.0*color.Y/(color.X + 15.0*color.Y + 3.0*color.Z);
  vnprime = 9.0*white.Y/(white.X + 15.0*white.Y + 3.0*white.Z);

  ret.L = 116.0*dmnsn_Lab_f(color.Y/white.Y) - 16.0;
  ret.u = 13.0*ret.L*(uprime - unprime);
  ret.v = 13.0*ret.L*(vprime - vnprime);

  return ret;
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

/* Convert a dmnsn_color to an sRGB color */
dmnsn_sRGB
dmnsn_sRGB_from_color(dmnsn_color color)
{
  double Rlinear, Glinear, Blinear; /* Linear RGB values - no gamma */
  dmnsn_sRGB ret;

  /*
   * First, the linear conversion.  Expressed as matrix multiplication, it looks
   * like this:
   *
   *   [Rlinear]   [ 3.2410 -1.5374 -0.4986] [X]
   *   [Glinear] = [-0.9692  1.8760  0.0416]*[Y]
   *   [Blinear]   [ 0.0556 -0.2040  1.0570] [Z]
   */
  Rlinear =  3.2410*color.X - 1.5374*color.Y - 0.4986*color.Z;
  Glinear = -0.9692*color.X + 1.8760*color.Y + 0.0416*color.Z;
  Blinear =  0.0556*color.X - 0.2040*color.Y + 1.0570*color.Z;

  ret.R = dmnsn_sRGB_C(Rlinear);
  ret.G = dmnsn_sRGB_C(Glinear);
  ret.B = dmnsn_sRGB_C(Blinear);

  return ret;
}

/* Add two colors in a perceptually correct manner, using CIE L*a*b*. */
dmnsn_color
dmnsn_color_add(dmnsn_color color1, dmnsn_color color2)
{
  dmnsn_CIE_Lab Lab1 = dmnsn_Lab_from_color(color1, dmnsn_whitepoint);
  dmnsn_CIE_Lab Lab2 = dmnsn_Lab_from_color(color2, dmnsn_whitepoint);

  dmnsn_CIE_Lab Lab = { .L = Lab1.L + Lab2.L,
                        .a = Lab1.a + Lab2.a,
                        .b = Lab1.b + Lab2.b };

  dmnsn_color ret = dmnsn_color_from_Lab(Lab, dmnsn_whitepoint);
  /* Weighted average of transparencies by intensity */
  if (Lab.L) {
    ret.filter = (Lab1.L*color1.filter + Lab2.L*color2.filter)/Lab.L;
    ret.trans  = (Lab1.L*color1.trans  + Lab2.L*color2.trans)/Lab.L;
  }

  return ret;
}

/* Multiply a color by a scalar */
dmnsn_color
dmnsn_color_mul(double n, dmnsn_color color)
{
  dmnsn_CIE_Lab Lab = dmnsn_Lab_from_color(color, dmnsn_whitepoint);
  Lab.L *= n;
  Lab.a *= n;
  Lab.b *= n;

  dmnsn_color ret = dmnsn_color_from_Lab(Lab, dmnsn_whitepoint);
  ret.filter = color.filter;
  ret.trans  = color.trans;

  return ret;
}

/* Filters `color' through `filter' */
dmnsn_color
dmnsn_color_filter(dmnsn_color color, dmnsn_color filter)
{
  dmnsn_color transmitted = dmnsn_color_mul(filter.trans, color);
  dmnsn_color filtered = dmnsn_color_mul(filter.filter,
                                         dmnsn_color_illuminate(filter, color));
  return dmnsn_color_add(transmitted, filtered);
}

/* Illuminates `color' with `light' */
dmnsn_color
dmnsn_color_illuminate(dmnsn_color light, dmnsn_color color)
{
  static dmnsn_CIE_RGB white = { 0.0 };
  if (!white.R)
    white = dmnsn_RGB_from_color(dmnsn_white);

  dmnsn_CIE_RGB RGB1 = dmnsn_RGB_from_color(light);
  dmnsn_CIE_RGB RGB2 = dmnsn_RGB_from_color(color);

  dmnsn_CIE_RGB RGB = {
    .R = RGB1.R*RGB2.R/white.R,
    .G = RGB1.G*RGB2.G/white.G,
    .B = RGB1.B*RGB2.B/white.B
  };

  dmnsn_color ret = dmnsn_color_from_RGB(RGB);
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
