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
  dmnsn_matrix trans;
  unsigned int i;

  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  scene = dmnsn_new_default_scene();
  if (!scene) {
    fprintf(stderr, "--- Couldn't create default scene! ---\n");
    return EXIT_FAILURE;
  }

  dmnsn_array_get(scene->objects, 0, &sphere);
  dmnsn_array_get(scene->objects, 1, &cube);
  trans = dmnsn_get_perspective_camera_trans(scene->camera);

  display = dmnsn_new_glX_display(scene->canvas);
  if (!display) {
    dmnsn_delete_default_scene(scene);
    fprintf(stderr, "--- Couldn't initialize X or glX! ---\n");
    return EXIT_FAILURE;
  }

  for (i = 0; i < 10; ++i) {
    progress = dmnsn_raytrace_scene_async(scene);
    if (!progress) {
      dmnsn_delete_display(display);
      dmnsn_delete_default_scene(scene);
      fprintf(stderr, "--- Couldn't start raytracing worker thread! ---\n");
      return EXIT_FAILURE;
    }

    progressbar("Raytracing scene: ", progress);

    if (dmnsn_finish_progress(progress) != 0) {
      dmnsn_delete_display(display);
      dmnsn_delete_default_scene(scene);
      fprintf(stderr, "--- Raytracing failed! ---\n");
      return EXIT_FAILURE;
    }

    if (dmnsn_gl_write_canvas(scene->canvas) != 0) {
      dmnsn_delete_display(display);
      dmnsn_delete_default_scene(scene);
      fprintf(stderr, "--- Drawing to openGL failed! ---\n");
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
  dmnsn_delete_default_scene(scene);
  return EXIT_SUCCESS;
}
