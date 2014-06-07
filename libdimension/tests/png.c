/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
main(void)
{
  int ret = EXIT_SUCCESS;

  // Treat warnings as errors for tests
  dmnsn_die_on_warnings(true);

  // Allocate our canvas
  dmnsn_pool *pool = dmnsn_new_pool();
  dmnsn_canvas *canvas = dmnsn_new_canvas(pool, 768, 480);

  // Optimize the canvas for PNG export
  if (dmnsn_png_optimize_canvas(canvas) != 0) {
    fprintf(stderr, "--- Couldn't optimize canvas for PNG! ---\n");
    ret = EXIT_FAILURE;
    goto exit;
  }

  // Paint the test pattern
  dmnsn_paint_test_canvas(canvas);

  // Write the image to PNG

  printf("Writing scene to PNG\n");
  FILE *ofile = fopen("png1.png", "wb");
  if (!ofile) {
    fprintf(stderr, "--- Couldn't open 'png1.png' for writing! ---\n");
    ret = EXIT_FAILURE;
    goto exit;
  }

  if (dmnsn_png_write_canvas(canvas, ofile) != 0) {
    fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
    fclose(ofile);
    ret = EXIT_FAILURE;
    goto exit;
  }

  fclose(ofile);

  /*
   * Now test PNG import/export
   */

  // Read the image back from PNG

  printf("Reading scene from PNG\n");
  FILE *ifile = fopen("png1.png", "rb");
  if (!ifile) {
    fprintf(stderr, "--- Couldn't open 'png1.png' for reading! ---\n");
    ret = EXIT_FAILURE;
    goto exit;
  }

  canvas = dmnsn_png_read_canvas(pool, ifile);
  if (!canvas) {
    fprintf(stderr, "--- Reading canvas from PNG failed! ---\n");
    fclose(ifile);
    ret = EXIT_FAILURE;
    goto exit;
  }

  fclose(ifile);

  // And write it back

  printf("Writing scene to PNG\n");
  ofile = fopen("png2.png", "wb");
  if (!ofile) {
    fprintf(stderr, "--- Couldn't open 'png2.png' for writing! ---\n");
    ret = EXIT_FAILURE;
    goto exit;
  }

  if (dmnsn_png_write_canvas(canvas, ofile) != 0) {
    fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
    fclose(ofile);
    ret = EXIT_FAILURE;
    goto exit;
  }

  fclose(ofile);

 exit:
  dmnsn_delete_pool(pool);
  return ret;
}
