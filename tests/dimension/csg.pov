/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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

// Test constructive solid geometry

camera {
  perspective
  location -4*z
  right    x*image_width/image_height
  look_at  0
}

background {
  color rgbf <0, 0.1, 0.2, 0.1>
}

/* One-object unions */

union {
  sphere {
    -1.5*x, 1
    pigment { color red 1 }
  }
}

union {
  light_source {
    20*y, color rgb 0.5
  }
}

/* CSG with lights */
difference {
  light_source {
    -15*x, color rgb 0.5
  }
  sphere {
    1.5*x - 20*y, 1
    pigment { color green 1 }
  }
  light_source {
    15*x, color rgb 0.5
  }
  box {
    <0.7, -20.8, -0.8>, <2.3, -19.2, 0.8>
    pigment { color blue 1 }
  }
  translate 20*y
}
