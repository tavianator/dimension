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

#include "tests.h"
#include "testsxx.hpp"
#include <fstream>
#include <iostream>

int
main() {
  using namespace Dimension;

  // Set the resilience low for tests
  resilience(SEVERITY_LOW);

  // Create the default test scene
  Scene scene = Tests::default_scene();

  // Create a glX window
  Tests::Display display(scene.canvas());

  {
    Raytracer raytracer(scene);
    GL_Writer writer(scene.canvas());

    // Render the scene
    Progress progress = raytracer.render_async();

    // Display the scene as it's rendered
    while (progress.progress() < 1.0) {
      writer.write();
      display.flush();
    }

    // Make sure we show the completed rendering
    progress.finish();
    writer.write();
    display.flush();
  }

  // Pause for a second
  sleep(1);

  // Read the canvas back from the GL buffer
  GL_Reader reader;
  Canvas canvas
    = reader.read(0, 0, scene.canvas().width(), scene.canvas().height());

  // And write it again
  GL_Writer writer(canvas);
  writer.write();
  display.flush();

  // Pause for a second
  sleep(1);

  return EXIT_SUCCESS;
}
