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

  // Get the camera
  Perspective_Camera& camera
    = dynamic_cast<Perspective_Camera&>(scene.camera());

  // Find the cube
  Cube* cube = 0;
  for (Scene::Iterator i = scene.begin(); i != scene.end(); ++i) {
    cube = dynamic_cast<Cube*>(&*i);
    if (cube) {
      break;
    }
  }
  if (!cube) {
    throw Dimension_Error("Couldn't find a cube in the default scene.");
  }

  Raytracer raytracer(scene);
  GL_Drawer drawer(scene.canvas());
  Tests::Display display(scene.canvas());

  // Render the animation
  const unsigned int frames = 10;
  for (unsigned int i = 0; i < frames; ++i) {
    // Render the scene
    Progress rprogress = raytracer.render_async();
    std::cout << "Raytracing scene: " << rprogress << std::endl;
    rprogress.finish();

    // Display the frame
    drawer.draw();
    display.flush();

    // Rotate the cube and camera for the next frame
    cube->trans(
      inverse(Matrix::rotation(Vector(0.025, 0.0, 0.0)))*cube->trans()
    );
    camera.trans(Matrix::rotation(Vector(0.0, -0.05, 0.0))*camera.trans());
  }

  return EXIT_SUCCESS;
}
