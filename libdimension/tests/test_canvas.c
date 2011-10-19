/*************************************************************************
 * Copyright (C) 2011 Tavian Barnes <tavianator@tavianator.com>          *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * The Dimension Test Suite is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License as *
 * published by the Free Software Foundation; either version 3 of the    *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Test Suite is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "tests.h"

void
dmnsn_paint_test_canvas(dmnsn_canvas *canvas)
{
  static const size_t size = 8;

  for (size_t y = 0; y < canvas->height; ++y) {
    for (size_t x = 0; x < canvas->width; ++x) {
      dmnsn_color a, b;
      double n = 3.0*x/canvas->width;
      if (x < canvas->width/3) {
        a = dmnsn_red;
        b = dmnsn_green;
      } else if (x < 2*canvas->width/3) {
        a = dmnsn_green;
        b = dmnsn_blue;
        n -= 1.0;
      } else {
        a = dmnsn_blue;
        b = dmnsn_red;
        n -= 2.0;
      }

      if ((x*y)%(2*size) < size) {
        a = dmnsn_color_to_sRGB(a);
        b = dmnsn_color_to_sRGB(b);
      }
      dmnsn_color color = dmnsn_color_gradient(a, b, n);
      if ((x*y)%(2*size) < size) {
        color = dmnsn_color_from_sRGB(color);
      }

      dmnsn_canvas_set_pixel(canvas, x, y, color);
    }
  }
}
