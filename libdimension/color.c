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
 * Color handling.
 */

#include "dimension.h"

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
const dmnsn_color dmnsn_clear = {
  .R = 0.0,
  .G = 0.0,
  .B = 0.0,
  .filter = 0.0,
  .trans  = 1.0
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
const dmnsn_color dmnsn_orange = {
  .R = 1.0,
  .G = 0.5,
  .B = 0.0,
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

  double n1 = dmnsn_color_intensity(c1), n2 = dmnsn_color_intensity(c2);
  if (n1 + n2 != 0.0) {
    ret.filter = (n1*c1.filter + n2*c2.filter)/(n1 + n2);
  }
  ret.trans = c1.trans + c2.trans;

  return ret;
}

/* Multiply a color by a scalar */
dmnsn_color
dmnsn_color_mul(double n, dmnsn_color color)
{
  color.R *= n;
  color.G *= n;
  color.B *= n;
  return color;
}

/* For n in [0, 1] get the color in a gradient between c1 and c2 */
dmnsn_color
dmnsn_color_gradient(dmnsn_color c1, dmnsn_color c2, double n)
{
  return dmnsn_new_color5(
    n*(c2.R - c1.R) + c1.R,
    n*(c2.G - c1.G) + c1.G,
    n*(c2.B - c1.B) + c1.B,
    n*(c2.filter - c1.filter) + c1.filter,
    n*(c2.trans  - c1.trans)  + c1.trans
  );
}

/* Filters `light' through `filter' */
dmnsn_color
dmnsn_filter_light(dmnsn_color light, dmnsn_color filter)
{
  dmnsn_color transmitted = dmnsn_color_mul(filter.trans, light);
  dmnsn_color filtered = dmnsn_color_mul(
    filter.filter,
    dmnsn_color_illuminate(filter, light)
  );
  dmnsn_color ret = dmnsn_color_add(transmitted, filtered);
  ret.filter = light.filter*dmnsn_color_intensity(filtered)
               + filter.filter*light.trans + filter.trans*light.filter;
  ret.trans  = filter.trans*light.trans;
  return ret;
}

/* Adds the background contribution, `filtered', to `filter' */
dmnsn_color
dmnsn_apply_translucency(dmnsn_color filtered, dmnsn_color filter)
{
  dmnsn_color ret = dmnsn_color_add(
    dmnsn_color_mul(1.0 - (filter.filter + filter.trans), filter),
    filtered
  );
  ret.filter = filtered.filter;
  ret.trans  = filtered.trans;
  return ret;
}

/* Adds the background contribution of `color' to `filter' */
dmnsn_color
dmnsn_apply_filter(dmnsn_color color, dmnsn_color filter)
{
  return dmnsn_apply_translucency(dmnsn_filter_light(color, filter), filter);
}

/* Illuminates `color' with `light' */
dmnsn_color
dmnsn_color_illuminate(dmnsn_color light, dmnsn_color color)
{
  return dmnsn_new_color5(light.R*color.R, light.G*color.G, light.B*color.B,
                          color.filter, color.trans);
}
