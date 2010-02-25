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

// Render demo scene

camera {
  perspective
  location <0, 0.25, -4>
  right    <1.6, 0, 0>
  look_at  <0, 0, 0>

  rotate   <0, 53, 0>
}

background {
  color rgbf <0, 0.1, 0.2, 0.1>
}

light_source {
  <-15, 20, 10>, color rgb <1, 1, 1>
}

box {
  <-1, -1, -1>, <1, 1, 1>

  rotate <45, 0, 0>

  texture {
    pigment {
      color rgbft <0, 0, 1, 0.25, 0.5>
    }
    finish {
      reflection { 0.5 }
    }
  }

  interior {
    ior 1.1
  }
}

sphere {
  <0, 0, 0>, 1.25

  texture {
    pigment {
      color rgb <0, 1, 0>
    }
    finish {
      phong 0.2
      phong_size 40.0
    }
  }
}
