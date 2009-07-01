/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
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

// Test C++ PNG file I/O

#include "testsxx.hpp"
#include <fstream>

int
main()
{
  using namespace Dimension;

  // Set the resilience low for tests
  resilience(SEVERITY_LOW);

  const unsigned int width = 333, height = 300;

  {
    std::ofstream ofstr("dimensionxx1.png", std::ios::binary);
    PNG_Canvas ocanvas(3*width, height, ofstr);

    CIE_xyY xyY;
    CIE_Lab Lab;
    CIE_Luv Luv;
    sRGB RGB;
    Color color;

    for (unsigned int x = 0; x < width; ++x) {
      for (unsigned int y = 0; y < height; ++y) {
        /* CIE xyY colorspace */
        xyY = CIE_xyY(static_cast<double>(x)/(width - 1),
                                 static_cast<double>(y)/(height - 1),
                                 0.5);
        color = xyY;
        RGB = color;

        if (RGB.R() > 1.0 || RGB.G() > 1.0 || RGB.B() > 1.0
            || RGB.R() < 0.0 || RGB.G() < 0.0 || RGB.B() < 0.0) {
          /* Out of sRGB gamut */
          color.trans(0.5);
        }

        ocanvas.pixel(x, y, color);

        /* CIE Lab colorspace */

        Lab = CIE_Lab(75.0,
                                 200.0*(static_cast<double>(x)/
                                        (width - 1) - 0.5),
                                 200.0*(static_cast<double>(y)/
                                        (height - 1) - 0.5));
        color = Lab;
        RGB = color;

        if (RGB.R() > 1.0 || RGB.G() > 1.0 || RGB.B() > 1.0
            || RGB.R() < 0.0 || RGB.G() < 0.0 || RGB.B() < 0.0) {
          /* Out of sRGB gamut */
          color.trans(0.5);
        }

        ocanvas.pixel(x + width, y, color);

        /* CIE Luv colorspace */

        Luv = CIE_Luv(75.0,
                                 200.0*(static_cast<double>(x)/
                                        (width - 1) - 0.5),
                                 200.0*(static_cast<double>(y)/
                                        (height - 1) - 0.5));
        color = Luv;
        RGB = color;

        if (RGB.R() > 1.0 || RGB.G() > 1.0 || RGB.B() > 1.0
            || RGB.R() < 0.0 || RGB.G() < 0.0 || RGB.B() < 0.0) {
          /* Out of sRGB gamut */
          color.trans(0.5);
        }

        ocanvas.pixel(x + 2*width, y, color);
      }
    }

    Progress progress = ocanvas.write_async();
    std::cout << "Writing PNG file: " << progress << std::endl;
  }

  std::ifstream ifstr("dimensionxx1.png", std::ios::binary);
  std::ofstream ofstr("dimensionxx2.png", std::ios::binary);

  Progress iprogress
    = PNG_Canvas::read_async(ifstr);
  std::cout << "Reading PNG file: " << iprogress << std::endl;

  PNG_Canvas iocanvas(iprogress, ofstr);

  Progress oprogress = iocanvas.write_async();
  std::cout << "Writing PNG file: " << oprogress << std::endl;

  return 0;
}
