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
  dmnsn_progress *progress;
  FILE *ifile, *ofile;
  dmnsn_scene *scene;
  dmnsn_canvas *canvas;

  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  /* Render the scene */
  {
    /* Allocate our default scene */
    scene = dmnsn_new_default_scene();
    if (!scene) {
      fprintf(stderr, "--- Allocation of default scene failed! ---\n");
      return EXIT_FAILURE;
    }

    /* Optimize the canvas for PNG export */
    if (dmnsn_png_optimize_canvas(scene->canvas) != 0) {
      dmnsn_delete_default_scene(scene);
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

    ofile = fopen("png1.png", "wb");
    if (!ofile) {
      dmnsn_delete_default_scene(scene);
      fprintf(stderr, "--- Couldn't open 'png1.png' for writing! ---\n");
      return EXIT_FAILURE;
    }

    progress = dmnsn_png_write_canvas_async(scene->canvas, ofile);
    if (!progress) {
      fclose(ofile);
      dmnsn_delete_default_scene(scene);
      fprintf(stderr, "--- Couldn't start PNG writing worker thread! ---\n");
      return EXIT_FAILURE;
    }

    progressbar("Writing PNG file: ", progress);

    if (dmnsn_finish_progress(progress) != 0) {
      fclose(ofile);
      dmnsn_delete_default_scene(scene);
      fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
      return EXIT_FAILURE;
    }

    fclose(ofile);
    dmnsn_delete_default_scene(scene);
  }

  /* Now test PNG import/export */
  {
    /* Read the image back from PNG */

    ifile = fopen("png1.png", "rb");
    if (!ifile) {
      fprintf(stderr, "--- Couldn't open 'png1.png' for reading! ---\n");
      return EXIT_FAILURE;
    }

    progress = dmnsn_png_read_canvas_async(&canvas, ifile);
    if (!progress) {
      fclose(ifile);
      fprintf(stderr, "--- Couldn't start PNG reading worker thread! ---\n");
      return EXIT_FAILURE;
    }

    progressbar("Reading PNG file: ", progress);

    if (dmnsn_finish_progress(progress) != 0) {
      fclose(ifile);
      fprintf(stderr, "--- Reading canvas from PNG failed! ---\n");
      return EXIT_FAILURE;
    }

    fclose(ifile);

    /* And write it back */

    ofile = fopen("png2.png", "wb");
    if (!ofile) {
      fprintf(stderr, "--- Couldn't open 'png2.png' for writing! ---\n");
      dmnsn_delete_canvas(canvas);
      return EXIT_FAILURE;
    }

    progress = dmnsn_png_write_canvas_async(canvas, ofile);
    if (!progress) {
      fclose(ofile);
      dmnsn_delete_canvas(canvas);
      fprintf(stderr, "--- Couldn't start PNG writing worker thread! ---\n");
      return EXIT_FAILURE;
    }

    progressbar("Writing PNG file: ", progress);

    if (dmnsn_finish_progress(progress) != 0) {
      fclose(ofile);
      dmnsn_delete_canvas(canvas);
      fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
      return EXIT_FAILURE;
    }

    fclose(ofile);
    dmnsn_delete_canvas(canvas);
  }

  return EXIT_SUCCESS;
}
