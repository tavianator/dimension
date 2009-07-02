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

#include "tests.h"
#include <stdlib.h>
#include <stdio.h>

int
main() {
  dmnsn_display *display;
  dmnsn_progress *progress;
  dmnsn_scene *scene;
  dmnsn_object *sphere, *cube;
  dmnsn_sRGB sRGB;
  dmnsn_color color;
  dmnsn_matrix trans;
  unsigned int i;

  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  /* Allocate our new scene */
  scene = dmnsn_new_scene();
  if (!scene) {
    fprintf(stderr, "--- Allocation of scene failed! ---\n");
    return EXIT_FAILURE;
  }

  /* Allocate a canvas */
  scene->canvas = dmnsn_new_canvas(768, 480);
  if (!scene->canvas) {
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Allocation of canvas failed! ---\n");
    return EXIT_FAILURE;
  }

  /* Set up the transformation matrix for the perspective camera */
  trans = dmnsn_scale_matrix(
    dmnsn_vector_construct(
      ((double)scene->canvas->x)/scene->canvas->y, 1.0, 1.0
    )
  );
  trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(dmnsn_vector_construct(0.0, 0.0, -4.0)),
    trans
  );
  trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_vector_construct(0.0, 1.0, 0.0)),
    trans
  );

  /* Create a perspective camera */
  scene->camera = dmnsn_new_perspective_camera(trans);
  if (!scene->camera) {
    dmnsn_delete_canvas(scene->canvas);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Allocation of camera failed! ---\n");
    return EXIT_FAILURE;
  }

  /* Background color */
  sRGB.R = 0.0;
  sRGB.G = 0.0;
  sRGB.B = 0.1;
  color = dmnsn_color_from_sRGB(sRGB);
  color.filter = 0.1;
  scene->background = color;

  /* Now make our objects */

  sphere = dmnsn_new_sphere();
  if (!sphere) {
    dmnsn_delete_perspective_camera(scene->camera);
    dmnsn_delete_canvas(scene->canvas);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Allocation of sphere failed! ---\n");
    return EXIT_FAILURE;
  }

  sphere->trans = dmnsn_matrix_inverse(
    dmnsn_scale_matrix(dmnsn_vector_construct(1.25, 1.25, 1.25))
  );
  dmnsn_array_push(scene->objects, &sphere);

  cube = dmnsn_new_cube();
  if (!cube) {
    dmnsn_delete_sphere(sphere);
    dmnsn_delete_perspective_camera(scene->camera);
    dmnsn_delete_canvas(scene->canvas);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Allocation of cube failed! ---\n");
    return EXIT_FAILURE;
  }

  cube->trans = dmnsn_matrix_inverse(
    dmnsn_rotation_matrix(dmnsn_vector_construct(0.75, 0.0, 0.0))
  );
  dmnsn_array_push(scene->objects, &cube);

  display = dmnsn_new_X_display(scene->canvas);
  if (!display) {
    dmnsn_delete_cube(cube);
    dmnsn_delete_sphere(sphere);
    dmnsn_delete_perspective_camera(scene->camera);
    dmnsn_delete_canvas(scene->canvas);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Couldn't initialize X! ---\n");
    return EXIT_FAILURE;
  }

  for (i = 0; i < 10; ++i) {
    progress = dmnsn_raytrace_scene_async(scene);
    if (!progress) {
      dmnsn_delete_display(display);
      dmnsn_delete_cube(cube);
      dmnsn_delete_sphere(sphere);
      dmnsn_delete_perspective_camera(scene->camera);
      dmnsn_delete_canvas(scene->canvas);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Couldn't start raytracing worker thread! ---\n");
      return EXIT_FAILURE;
    }

    progressbar("Raytracing scene: ", progress);

    if (dmnsn_finish_progress(progress) != 0) {
      dmnsn_delete_display(display);
      dmnsn_delete_cube(cube);
      dmnsn_delete_sphere(sphere);
      dmnsn_delete_perspective_camera(scene->camera);
      dmnsn_delete_canvas(scene->canvas);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Raytracing failed! ---\n");
      return EXIT_FAILURE;
    }

    if (dmnsn_X_write_canvas(scene->canvas) != 0) {
      dmnsn_delete_display(display);
      dmnsn_delete_cube(cube);
      dmnsn_delete_sphere(sphere);
      dmnsn_delete_perspective_camera(scene->camera);
      dmnsn_delete_canvas(scene->canvas);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Drawing to X failed! ---\n");
      return EXIT_FAILURE;
    }
    dmnsn_display_frame(display);

    /* Rotate the cube and camera for the next frame */

    cube->trans = dmnsn_matrix_mul(
      dmnsn_matrix_inverse(
        dmnsn_rotation_matrix(dmnsn_vector_construct(0.025, 0.0, 0.0))
      ),
      cube->trans
    );

    trans = dmnsn_matrix_mul(
      dmnsn_rotation_matrix(dmnsn_vector_construct(0.0, -0.05, 0.0)),
      trans
    );
    dmnsn_set_perspective_camera_trans(scene->camera, trans);
  }

  dmnsn_delete_display(display);
  dmnsn_delete_cube(cube);
  dmnsn_delete_sphere(sphere);
  dmnsn_delete_perspective_camera(scene->camera);
  dmnsn_delete_canvas(scene->canvas);
  dmnsn_delete_scene(scene);
  return EXIT_SUCCESS;
}
