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
#include <math.h>
#include <stdlib.h>

/* Conversions between CIE 1931 XYZ and sRGB color. */

dmnsn_pixel
dmnsn_pixel_from_color(dmnsn_color color)
{
  double Rlinear, Glinear, Blinear; /* Linear RGB values - no gamma */
  double R, G, B;                   /* sRGB values */
  dmnsn_pixel pixel;

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

  /*
   * If C represents R, G, and B, then the sRGB values are now found as follows:
   *
   *         { 12.92*Clinear,                Clinear <= 0.0031308
   * Csrgb = {                1/2.4
   *         { (1.055)*Clinear      - 0.055, Clinear >  0.0031308
   */

  if (Rlinear <= 0.0031308) {
    R = 12.92*Rlinear;
  } else {
    R = 1.055*pow(Rlinear, 1.0/2.4) - 0.055;
  }

  if (Glinear <= 0.0031308) {
    G = 12.92*Glinear;
  } else {
    G = 1.055*pow(Glinear, 1.0/2.4) - 0.055;
  }

  if (Blinear <= 0.0031308) {
    B = 12.92*Blinear;
  } else {
    B = 1.055*pow(Blinear, 1.0/2.4) - 0.055;
  }

  /* Now we go from unlimited to limited light, saturating at UINT16_MAX */

  if (R < 0) {
    pixel.r = 0;
  } else if (R > 1) {
    pixel.r = UINT16_MAX;
  } else {
    pixel.r = UINT16_MAX*R;
  }

  if (G < 0) {
    pixel.g = 0;
  } else if (G > 1) {
    pixel.g = UINT16_MAX;
  } else {
    pixel.g = UINT16_MAX*G;
  }

  if (B < 0) {
    pixel.b = 0;
  } else if (B > 1) {
    pixel.b = UINT16_MAX;
  } else {
    pixel.b = UINT16_MAX*B;
  }

  if (color.alpha < 0) {
    pixel.a = 0;
  } else if (color.alpha > 1) {
    pixel.a = UINT16_MAX;
  } else {
    pixel.a = UINT16_MAX*color.alpha;
  }

  if (color.trans < 0) {
    pixel.t = 0;
  } else if (color.trans > 1) {
    pixel.t = UINT16_MAX;
  } else {
    pixel.t = UINT16_MAX*color.trans;
  }

  return pixel;
}

dmnsn_color
dmnsn_color_from_pixel(dmnsn_pixel pixel)
{
  double R, G, B;                   /* sRGB values */
  double Rlinear, Glinear, Blinear; /* Linear RGB values - no gamma */
  dmnsn_color color;

  /* Conversion back to unlimited light */
  R = ((double)pixel.r)/UINT16_MAX;
  G = ((double)pixel.g)/UINT16_MAX;
  B = ((double)pixel.b)/UINT16_MAX;

  /*
   * If C represents R, G, and B, then the Clinear values are now found as
   * follows:
   *
   *           { Csrgb/12.92,                  Csrgb <= 0.04045
   * Clinear = {                        1/2.4
   *           { ((Csrgb + 0.055)/1.055)     , Csrgb >  0.04045
   */

  if (R <= 0.04045) {
    Rlinear = R/19.92;
  } else {
    Rlinear = pow((R + 0.055)/1.055, 2.4);
  }

  if (G <= 0.04045) {
    Glinear = G/19.92;
  } else {
    Glinear = pow((G + 0.055)/1.055, 2.4);
  }

  if (B <= 0.04045) {
    Blinear = B/19.92;
  } else {
    Blinear = pow((B + 0.055)/1.055, 2.4);
  }

  /*
   * Now, the linear conversion.  Expressed as matrix multiplication, it looks
   * like this:
   *
   *   [X]   [0.4124 0.3576 0.1805] [Rlinear]
   *   [Y] = [0.2126 0.7152 0.0722]*[Glinear]
   *   [X]   [0.0193 0.1192 0.9505] [Blinear]
   */

  color.X = 0.4124*Rlinear + 0.3576*Glinear + 0.1805*Blinear;
  color.Y = 0.2126*Rlinear + 0.7152*Glinear + 0.0722*Blinear;
  color.Z = 0.0193*Rlinear + 0.1192*Glinear + 0.9505*Blinear;

  color.alpha = ((double)pixel.a)/UINT16_MAX;
  color.trans = ((double)pixel.t)/UINT16_MAX;

  return color;
}

dmnsn_canvas *
dmnsn_new_canvas(unsigned int x, unsigned int y)
{
  dmnsn_canvas *canvas = malloc(sizeof(dmnsn_canvas));

  if (canvas) {
    canvas->x = x;
    canvas->y = y;
    canvas->pixels = malloc(sizeof(dmnsn_pixel)*x*y);

    if (canvas->pixels) {
      return canvas;
    } else {
      free(canvas);
      return NULL;
    }
  } else {
    return NULL;
  }
}

void
dmnsn_delete_canvas(dmnsn_canvas *canvas)
{
  if (canvas) {
    free(canvas->pixels);
    free(canvas);
  }
}
