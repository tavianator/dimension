/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

int
main() {
  bool have_png = true, have_gl = true;

  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  /* Create the default test scene */
  dmnsn_scene *scene = dmnsn_new_default_scene();

  /* Optimize the canvas for PNG export */
  errno = 0;
  if (dmnsn_png_optimize_canvas(scene->canvas) != 0) {
    if (errno == ENOTSUP) {
      have_png = false;
    } else {
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Couldn't optimize canvas for PNG! ---\n");
      return EXIT_FAILURE;
    }
  }

  /* Optimize the canvas for GL drawing */
  errno = 0;
  if (dmnsn_gl_optimize_canvas(scene->canvas) != 0) {
    if (errno == ENOTSUP) {
      have_gl = false;
    } else {
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Couldn't optimize canvas for GL! ---\n");
      return EXIT_FAILURE;
    }
  }

  dmnsn_clear_canvas(scene->canvas, dmnsn_black);

  /* Create a new glX display */
  dmnsn_display *display = NULL;
  if (have_gl) {
    display = dmnsn_new_display(scene->canvas);
    if (!display) {
      fprintf(stderr, "--- WARNING: Couldn't initialize X or glX! ---\n");
    }
  }

  /* Render the scene */

  printf("Rendering scene\n");
  dmnsn_progress *progress = dmnsn_raytrace_scene_async(scene);

  /* Display the scene as it's rendered */
  if (display) {
    while (dmnsn_get_progress(progress) < 1.0) {
      if (dmnsn_gl_write_canvas(scene->canvas) != 0) {
        dmnsn_delete_display(display);
        dmnsn_delete_scene(scene);
        fprintf(stderr, "--- Drawing to OpenGL failed! ---\n");
        return EXIT_FAILURE;
      }
      dmnsn_display_flush(display);
    }
  }

  if (dmnsn_finish_progress(progress) != 0) {
    dmnsn_delete_display(display);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Raytracing failed! ---\n");
    return EXIT_FAILURE;
  }

  /* Make sure we show the completed rendering */
  if (display) {
    printf("Drawing to OpenGL\n");
    if (dmnsn_gl_write_canvas(scene->canvas) != 0) {
      dmnsn_delete_display(display);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Drawing to OpenGL failed! ---\n");
      return EXIT_FAILURE;
    }
    dmnsn_display_flush(display);
  }

  if (have_png) {
    printf("Writing scene to PNG\n");
    FILE *ofile = fopen("render.png", "wb");
    if (!ofile) {
      dmnsn_delete_display(display);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Couldn't open 'render.png' for writing! ---\n");
      return EXIT_FAILURE;
    }

    if (dmnsn_png_write_canvas(scene->canvas, ofile) != 0) {
      fclose(ofile);
      dmnsn_delete_display(display);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
      return EXIT_FAILURE;
    }

    fclose(ofile);
  }

  dmnsn_delete_display(display);
  dmnsn_delete_scene(scene);
  return EXIT_SUCCESS;
}
