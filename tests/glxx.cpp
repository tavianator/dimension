/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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

  // Background color
  Color background = sRGB(0.0, 0.1, 0.25);
  background.filter(0.1);

  // Canvas
  Canvas canvas(768, 480);

  // Camera
  Perspective_Camera camera(
    Matrix::rotation(Vector(0.0, 1.0, 0.0))
    * Matrix::translation(Vector(0.0, 0.0, -4.0))
    * Matrix::scale(
        Vector(static_cast<double>(canvas.width())/canvas.height(), 1.0, 1.0)
      )
  );

  // Scene
  Scene scene(background, camera, canvas);

  // Objects in scene

  Sphere sphere;
  sphere.trans(inverse(Matrix::scale(Vector(1.25, 1.25, 1.25))));
  scene.push_object(sphere);

  Cube cube;
  cube.trans(inverse(Matrix::rotation(Vector(0.75, 0.0, 0.0))));
  scene.push_object(cube);

  Raytracer raytracer(scene);
  GL_Drawer drawer(scene.canvas());
  Dimension::Display display(scene.canvas());

  // Render the scene
  for (unsigned int i = 0; i < 10; ++i) {
    Progress rprogress = raytracer.render_async();
    std::cout << "Raytracing scene: " << rprogress << std::endl;
    rprogress.finish();

    drawer.draw();
    display.flush();

    cube.trans(inverse(Matrix::rotation(Vector(0.025, 0.0, 0.0)))*cube.trans());
    camera.trans(Matrix::rotation(Vector(0.0, -0.05, 0.0))*camera.trans());
  }

  return EXIT_SUCCESS;
}
