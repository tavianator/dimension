/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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
main()
{
  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  /* Allocate our canvas */
  dmnsn_canvas *canvas = dmnsn_new_canvas(768, 480);

  /* Optimize the canvas for PNG export */
  if (dmnsn_png_optimize_canvas(canvas) != 0) {
    dmnsn_delete_canvas(canvas);
    fprintf(stderr, "--- Couldn't optimize canvas for PNG! ---\n");
    return EXIT_FAILURE;
  }

  /* Paint the canvas blue */
  dmnsn_clear_canvas(canvas, dmnsn_blue);

  /* Write the image to PNG */

  printf("Writing scene to PNG\n");
  FILE *ofile = fopen("png1.png", "wb");
  if (!ofile) {
    dmnsn_delete_canvas(canvas);
    fprintf(stderr, "--- Couldn't open 'png1.png' for writing! ---\n");
    return EXIT_FAILURE;
  }

  if (dmnsn_png_write_canvas(canvas, ofile) != 0) {
    fclose(ofile);
    dmnsn_delete_canvas(canvas);
    fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
    return EXIT_FAILURE;
  }

  fclose(ofile);
  dmnsn_delete_canvas(canvas);

  /*
   * Now test PNG import/export
   */

  /* Read the image back from PNG */

  printf("Reading scene from PNG\n");
  FILE *ifile = fopen("png1.png", "rb");
  if (!ifile) {
    fprintf(stderr, "--- Couldn't open 'png1.png' for reading! ---\n");
    return EXIT_FAILURE;
  }

  canvas = dmnsn_png_read_canvas(ifile);
  if (!canvas) {
    fclose(ifile);
    fprintf(stderr, "--- Reading canvas from PNG failed! ---\n");
    return EXIT_FAILURE;
  }

  fclose(ifile);

  /* And write it back */

  printf("Writing scene to PNG\n");
  ofile = fopen("png2.png", "wb");
  if (!ofile) {
    fprintf(stderr, "--- Couldn't open 'png2.png' for writing! ---\n");
    dmnsn_delete_canvas(canvas);
    return EXIT_FAILURE;
  }

  if (dmnsn_png_write_canvas(canvas, ofile) != 0) {
    fclose(ofile);
    dmnsn_delete_canvas(canvas);
    fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
    return EXIT_FAILURE;
  }

  fclose(ofile);
  dmnsn_delete_canvas(canvas);

  return EXIT_SUCCESS;
}
