/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of Dimension.                                       *
 *                                                                       *
 * Dimension is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU Lesser General Public License as published *
 * by the Free Software Foundation; either version 3 of the License, or  *
 * (at your option) any later version.                                   *
 *                                                                       *
 * Dimension is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include "dimension.h"
#include <math.h> /* For pow() */

dmnsn_CIE_XYZ whitepoint = { 0.9505, 1, 1.089 };

dmnsn_color
dmnsn_color_from_XYZ(dmnsn_CIE_XYZ XYZ)
{
  dmnsn_color ret;
  ret.X = XYZ.X;
  ret.Y = XYZ.Y;
  ret.Z = XYZ.Z;
  ret.filter = 0.0;
  ret.trans  = 0.0;
  return ret;
}

dmnsn_color
dmnsn_color_from_xyY(dmnsn_CIE_xyY xyY)
{
  dmnsn_color ret;
  ret.X = xyY.Y*xyY.x/xyY.y;
  ret.Y = xyY.Y;
  ret.Z = xyY.Y*(1.0 - xyY.x - xyY.Y)/xyY.y;
  ret.filter = 0.0;
  ret.trans  = 0.0;
  return ret;
}

static double Lab_finv(double t) {
  if (t > 6.0/29.0) {
    return t*t*t;
  } else {
    return 108.0*(t - 16.0/116.0)/841.0;
  }
}

dmnsn_color
dmnsn_color_from_Lab(dmnsn_CIE_Lab Lab, dmnsn_CIE_XYZ white)
{
  double fx, fy, fz;
  dmnsn_color ret;

  fy = (Lab.L + 16.0)/116.0;
  fx = fy + Lab.a/500.0;
  fz = fy - Lab.b/200.0;

  ret.X = white.X*Lab_finv(fx);
  ret.Y = white.Y*Lab_finv(fy);
  ret.Z = white.Z*Lab_finv(fz);

  return ret;
}

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

  ret.Y = white.Y*Lab_finv(fy);
  ret.X = ret.Y*9.0*uprime/vprime/4.0;
  ret.Z = ret.Y*(12.0 - 3*uprime - 20*vprime)/vprime/4.0;

  return ret;
}

static double sRGB_Cinv(double CsRGB) {
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

dmnsn_color
dmnsn_color_from_sRGB(dmnsn_sRGB sRGB)
{
  double Rlinear, Glinear, Blinear; /* Linear RGB values - no gamma */
  dmnsn_color ret;

  Rlinear = sRGB_Cinv(sRGB.R);
  Glinear = sRGB_Cinv(sRGB.G);
  Blinear = sRGB_Cinv(sRGB.B);

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

dmnsn_CIE_XYZ
dmnsn_XYZ_from_color(dmnsn_color color)
{
  dmnsn_CIE_XYZ ret = { color.X, color.Y, color.Z };
  return ret;
}

dmnsn_CIE_xyY
dmnsn_xyY_from_color(dmnsn_color color)
{
  dmnsn_CIE_xyY ret = { color.X/(color.X + color.Y + color.Z),
                        color.Y/(color.X + color.Y + color.Z),
                        color.Y };
  return ret;
}

static double Lab_f(double t) {
  if (t > 216.0/24389.0) {
    return pow(t, 1.0/3.0);
  } else {
    return 841.0*t/108.0 + 4.0/29.0;
  }
}

dmnsn_CIE_Lab
dmnsn_Lab_from_color(dmnsn_color color, dmnsn_CIE_XYZ white)
{
  dmnsn_CIE_Lab ret;

  ret.L = 116.0*Lab_f(color.Y/white.Y) - 16.0;
  ret.a = 500.0*(Lab_f(color.X/white.X) - Lab_f(color.Y/white.Y));
  ret.b = 200.0*(Lab_f(color.Y/white.Y) - Lab_f(color.Z/white.Z));

  return ret;
}

dmnsn_CIE_Luv
dmnsn_Luv_from_color(dmnsn_color color, dmnsn_CIE_XYZ white)
{
  double uprime, unprime, vprime, vnprime;
  dmnsn_CIE_Luv ret;

  uprime  = 4.0*color.X/(color.X + 15.0*color.Y + 3.0*color.Z);
  unprime = 4.0*white.X/(white.X + 15.0*white.Y + 3.0*white.Z);
  vprime  = 9.0*color.Y/(color.X + 15.0*color.Y + 3.0*color.Z);
  vnprime = 9.0*white.Y/(white.X + 15.0*white.Y + 3.0*white.Z);

  ret.L = 116.0*Lab_f(color.Y/white.Y) - 16.0;
  ret.u = 13.0*ret.L*(uprime - unprime);
  ret.v = 13.0*ret.L*(vprime - vnprime);

  return ret;
}

static double sRGB_C(double Clinear) {
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

  ret.R = sRGB_C(Rlinear);
  ret.G = sRGB_C(Glinear);
  ret.B = sRGB_C(Blinear);

  return ret;
}

dmnsn_color
dmnsn_color_add(dmnsn_color color1, dmnsn_color color2)
{
  dmnsn_CIE_Lab Lab, Lab1, Lab2;
  dmnsn_color ret;

  Lab1 = dmnsn_Lab_from_color(color1, whitepoint);
  Lab2 = dmnsn_Lab_from_color(color2, whitepoint);

  Lab.L = Lab1.L + Lab2.L;
  Lab.a = (Lab1.L*Lab1.a + Lab2.L*Lab2.a)/Lab.L;
  Lab.b = (Lab1.L*Lab1.b + Lab2.L*Lab2.b)/Lab.L;

  ret = dmnsn_color_from_Lab(Lab, whitepoint);
  ret.filter = (Lab1.L*color1.filter + Lab2.L*color2.filter)/Lab.L;
  ret.trans  = (Lab1.L*color1.trans  + Lab2.L*color2.trans)/Lab.L;

  return ret;
}
