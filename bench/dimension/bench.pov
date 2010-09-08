/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
 *                                                                       *
 * This file is part of The Dimension Benchmark Suite.                   *
 *                                                                       *
 * The Dimension Benchmark Suite is free software; you can redistribute  *
 * it and/or modify it under the terms of the GNU General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Benchmark Suite is distributed in the hope that it will *
 * be useful, but WITHOUT ANY WARRANTY; without even the implied         *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See *
 * the GNU General Public License for more details.                      *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

camera {
  location  <3.0, 6.0, -11.0>
  right     x*image_width/image_height
  look_at   0
}

background {
  color rgb 1
}

// inside center sphere
light_source {
  0,
  color rgb 1
}

light_source {
  2*y,
  color rgb 1
}

/* plane {
  y,
  -1
  // *** hollow on
  pigment {
    rgb <0.73, 0.90, 0.97>
  }
  finish {
    diffuse 0.35
    ambient .5
  }
} */

#macro sph(center)
  sphere {
    center,
    1
    texture {
      // *** crackle
      scale 0.5

      /* *** texture_map {
        [ 0.03
          pigment {
            color rgb 1
          }
          finish {
            ambient 1
          }
          normal {
            facets size 0.1
          }
        ]
        [ 0.04
          pigment {
            color rgbf <1, 1, 1, 0.9>
          }
          finish {
            reflection { 0.2 }
            specular 0.1
            roughness 0.02
            conserve_energy
          }
          normal {
            facets size 0.1
          }
        ]
      } *** */
    }
    interior {
      ior 1.3
    }
  }
#end

union {
  #declare Size = 4;
  #declare I = -Size;
  #while (I <= Size)
    #declare J = -Size;

    #while (J <= Size)
      #declare K = -Size;

      #while (K <= Size)
        object {
          sph(<2.5*I, 2.5*K, 2.5*J>)
        }

        #declare K = K + 1;
      #end

      #declare J = J + 1;
    #end

    #declare I = I + 1;
  #end
}
