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
#include <stdlib.h>
#include <stdio.h>

int
main() {
  dmnsn_progress *progress;
  FILE *file;
  dmnsn_scene *scene;

  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  /* Allocate our default scene */
  scene = dmnsn_new_default_scene();
  if (!scene) {
    fprintf(stderr, "--- Allocation of default scene failed! ---\n");
    return EXIT_FAILURE;
  }

  /* Optimize the canvas for PNG export */
  if (dmnsn_png_optimize_canvas(canvas) != 0) {
    dmnsn_delete_canvas(canvas);
    fprintf(stderr, "--- Couldn't optimize canvas for PNG! ---\n");
    return EXIT_FAILURE;
  }

  /* Render scene */

  progress = dmnsn_raytrace_scene_async(scene);
  if (!progress) {
    dmnsn_delete_default_scene(scene);
    fprintf(stderr, "--- Couldn't start raytracing worker thread! ---\n");
    return EXIT_FAILURE;
  }

  progressbar("Raytracing scene: ", progress);

  if (dmnsn_finish_progress(progress) != 0) {
    dmnsn_delete_default_scene(scene);
    fprintf(stderr, "--- Raytracing failed! ---\n");
    return EXIT_FAILURE;
  }

  /* Write the image to PNG */

  file = fopen("raytrace.png", "wb");
  if (!file) {
    dmnsn_delete_default_scene(scene);
    fprintf(stderr, "--- Couldn't open 'raytrace.png' for writing! ---\n");
    return EXIT_FAILURE;
  }

  progress = dmnsn_png_write_canvas_async(scene->canvas, file);
  if (!progress) {
    fclose(file);
    dmnsn_delete_default_scene(scene);
    fprintf(stderr, "--- Couldn't start PNG writing worker thread! ---\n");
    return EXIT_FAILURE;
  }

  progressbar("Writing PNG file: ", progress);

  if (dmnsn_finish_progress(progress) != 0) {
    fclose(file);
    dmnsn_delete_default_scene(scene);
    fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
    return EXIT_FAILURE;
  }

  fclose(file);
  dmnsn_delete_default_scene(scene);
  return EXIT_SUCCESS;
}
