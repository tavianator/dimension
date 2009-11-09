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
#include <stdlib.h>
#include <stdio.h>

int
main() {
  dmnsn_display *display;
  dmnsn_progress *progress;
  dmnsn_scene *scene;
  dmnsn_canvas *canvas;

  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  /* Create the default test scene */
  scene = dmnsn_new_default_scene();
  if (!scene) {
    fprintf(stderr, "--- Couldn't create default scene! ---\n");
    return EXIT_FAILURE;
  }

  /* Optimize the canvas for GL drawing */
  if (dmnsn_gl_optimize_canvas(scene->canvas) != 0) {
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Couldn't optimize canvas for GL! ---\n");
    return EXIT_FAILURE;
  }

  /* Create a new glX display */
  display = dmnsn_new_display(scene->canvas);
  if (!display) {
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- WARNING: Couldn't initialize X or glX! ---\n");
    return EXIT_SUCCESS;
  }

  /* Render the scene */

  printf("Rendering scene\n");
  progress = dmnsn_raytrace_scene_async(scene);
  if (!progress) {
    dmnsn_delete_display(display);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Couldn't start raytracing worker thread! ---\n");
    return EXIT_FAILURE;
  }

  /* Display the scene as it's rendered */
  while (dmnsn_get_progress(progress) < 1.0) {
    if (dmnsn_gl_write_canvas(scene->canvas) != 0) {
      dmnsn_delete_display(display);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Drawing to openGL failed! ---\n");
      return EXIT_FAILURE;
    }
    dmnsn_display_flush(display);
  }

  if (dmnsn_finish_progress(progress) != 0) {
    dmnsn_delete_display(display);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Raytracing failed! ---\n");
    return EXIT_FAILURE;
  }

  /* Make sure we show the completed rendering */
  printf("Drawing to OpenGL\n");
  if (dmnsn_gl_write_canvas(scene->canvas) != 0) {
    dmnsn_delete_display(display);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Drawing to openGL failed! ---\n");
    return EXIT_FAILURE;
  }
  dmnsn_display_flush(display);

  /* Show the image on screen for a bit */
  sleep(1);

  /* Read a canvas from the GL buffer */
  printf("Reading from OpenGL\n");
  canvas = dmnsn_gl_read_canvas(0, 0, scene->canvas->x, scene->canvas->y);
  if (!canvas) {
    dmnsn_delete_display(display);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Reading canvas from GL buffer failed! ---\n");
    return EXIT_FAILURE;
  }

  /* And write it back */
  printf("Drawing to OpenGL\n");
  if (dmnsn_gl_write_canvas(canvas) != 0) {
    dmnsn_delete_canvas(canvas);
    dmnsn_delete_display(display);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Drawing to openGL failed! ---\n");
    return EXIT_FAILURE;
  }
  dmnsn_display_flush(display);

  /* Show the image on screen for a bit */
  sleep(1);

  dmnsn_delete_canvas(canvas);
  dmnsn_delete_display(display);
  dmnsn_delete_scene(scene);
  return EXIT_SUCCESS;
}
