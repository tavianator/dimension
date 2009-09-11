/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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

#include "tests.hpp"
#include <fstream>
#include <iostream>

int
main() {
  using namespace Dimension;

  // Set the resilience low for tests
  resilience(SEVERITY_LOW);

  // Perform the rendering
  {
    // Get the default test scene
    Scene scene = Tests::default_scene();

    std::ofstream ofile("pngxx1.png");
    PNG_Writer writer(scene.canvas(), ofile);

    // Render the scene
    Raytracer raytracer(scene);
    Progress rprogress = raytracer.render_async();
    std::cout << "Raytracing scene: " << rprogress << std::endl;
    rprogress.finish();

    // Write the canvas
    Progress oprogress = writer.write_async();
    std::cout << "Writing PNG file: " << oprogress << std::endl;
  }

  // Now test PNG import/export
  {
    std::ifstream ifile("pngxx1.png");
    PNG_Reader reader(ifile);
    Progress iprogress = reader.read_async();
    std::cout << "Reading PNG file: " << iprogress << std::endl;

    Canvas canvas = PNG_Reader::finish(iprogress);
    std::ofstream ofile("pngxx2.png");
    PNG_Writer writer(canvas, ofile);
    Progress oprogress = writer.write_async();
    std::cout << "Writing PNG file: " << oprogress << std::endl;
  }

  return EXIT_SUCCESS;
}
