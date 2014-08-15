/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Colors with transparency information.
 */

/** A transparent color. */
typedef struct dmnsn_tcolor {
  dmnsn_color c; /**< Color. */
  double T;      /**< Transparency. */
  double F;      /**< Proportion of filtered transparency. */
} dmnsn_tcolor;

/** A standard format string for colors. */
#define DMNSN_TCOLOR_FORMAT "TColor<%g, %g, %g, %g, %g>"
/** The appropriate arguements to printf() a color. */
#define DMNSN_TCOLOR_PRINTF(tc) (tc).c.R, (tc).c.G, (tc).c.B, (tc).T, (tc).F

/** Create a tcolor. */
DMNSN_INLINE dmnsn_tcolor
dmnsn_new_tcolor(dmnsn_color c, double T, double F)
{
  dmnsn_tcolor ret = { c, T, F };
  return ret;
}

/** Convert a dmnsn_color into a dmnsn_tcolor. */
#define DMNSN_TCOLOR(c) dmnsn_new_tcolor(c, 0.0, 0.0)

/** Create a tcolor with individually-specified components. */
DMNSN_INLINE dmnsn_tcolor
dmnsn_new_tcolor5(double R, double G, double B, double T, double F)
{
  dmnsn_tcolor ret = { dmnsn_new_color(R, G, B), T, F };
  return ret;
}

/** Return the color at \p n on a gradient from \p c1 at 0 to \p c2 at 1. */
DMNSN_INLINE dmnsn_tcolor
dmnsn_tcolor_gradient(dmnsn_tcolor c1, dmnsn_tcolor c2, double n)
{
  return dmnsn_new_tcolor(
    dmnsn_color_gradient(c1.c, c2.c, n),
    n*(c2.T - c1.T) + c1.T,
    n*(c2.F - c1.F) + c1.F
  );
}

/** Filter \p light through \p filter. */
DMNSN_INLINE dmnsn_color
dmnsn_tcolor_filter(dmnsn_color light, dmnsn_tcolor filter)
{
  dmnsn_color filtered =
    dmnsn_color_mul(filter.T*filter.F, dmnsn_color_illuminate(light, filter.c));
  dmnsn_color transmitted = dmnsn_color_mul(filter.T*(1.0 - filter.F), light);
  return dmnsn_color_add(filtered, transmitted);
}

/** Remove the filtered component of a tcolor. */
DMNSN_INLINE dmnsn_tcolor
dmnsn_tcolor_remove_filter(dmnsn_tcolor tcolor)
{
  double intensity = dmnsn_color_intensity(tcolor.c);
  double newtrans = (1.0 - (1.0 - intensity)*tcolor.F)*tcolor.T;
  if (1.0 - newtrans >= dmnsn_epsilon) {
    tcolor.c = dmnsn_color_mul((1.0 - tcolor.T)/(1.0 - newtrans), tcolor.c);
  }
  tcolor.T = newtrans;
  tcolor.F = 0.0;
  return tcolor;
}

/** Saturate the tcolor components to [0.0, 1.0]. */
DMNSN_INLINE dmnsn_tcolor
dmnsn_tcolor_clamp(dmnsn_tcolor tcolor)
{
  tcolor.c = dmnsn_color_clamp(tcolor.c);
  tcolor.T = dmnsn_min(dmnsn_max(tcolor.T, 0.0), 1.0);
  tcolor.F = dmnsn_min(dmnsn_max(tcolor.F, 0.0), 1.0);
  return tcolor;
}

/** Return whether a tcolor contains any NaN components. */
DMNSN_INLINE bool
dmnsn_tcolor_isnan(dmnsn_tcolor tcolor)
{
  return dmnsn_color_isnan(tcolor.c) || dmnsn_isnan(tcolor.T) || dmnsn_isnan(tcolor.F);
}

/* Standard tcolors */

/** Clear. */
#define dmnsn_clear dmnsn_new_tcolor5(0.0, 0.0, 0.0, 1.0, 0.0)
