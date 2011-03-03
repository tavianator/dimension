/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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
  right    x*image_width/image_height
  look_at  <0, 0, 0>

  rotate   53*y
}

background {
  color transmit 1
}

sky_sphere {
  pigment {
    gradient y
    color_map {
      [0.0   color rgb <1, 0.5, 0>]
      [0.35  color rgbf <0, 0.1, 0.2, 0.1>]
    }
  }
}

light_source {
  <-15, 20, 10>, color rgb <1, 1, 1>
}

difference {
  box {
    <-1, -1, -1>, <1, 1, 1>

    rotate 45*x

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
}

union {
  cylinder {
    -1.25*y, 1.25*y, 0.1
  }
  cone {
    1.25*y, 0.1, 1.5*y, 0
    open
  }

  pigment {
    gradient y
    color_map {
      [0    color rgb <1, 0, 0>]
      [1/6  color rgb <1, 0.5, 0>]
      [2/6  color rgb <1, 1, 0>]
      [3/6  color rgb <0, 1, 0>]
      [4/6  color rgb <0, 0, 1>]
      [5/6  color rgb <1, 0, 1>]
      [1    color rgb <1, 0, 0>]
    }
    scale <1, 2.75, 1>
    translate -1.25*y
  }
  rotate -45*x
}

union {
  torus {
    0.15, 0.05
    translate -y
  }
  torus {
    0.15, 0.05
  }
  torus {
    0.15, 0.05
    translate y
  }

  pigment {
    color rgb <0, 0, 1>
  }
  finish {
    ambient 1
  }
  rotate -45*x
}

plane {
  y, -2
  pigment {
    checker
    pigment {
      color rgb 1
    }
    pigment {
      checker color rgb 0, color rgb 1
      scale 1/3
    }
    quick_color rgb <1, 0.5, 0.75>
  }
}
