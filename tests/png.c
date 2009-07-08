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

/* Test PNG file I/O */

#include "tests.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int
main() {
  dmnsn_canvas *canvas;
  dmnsn_progress *progress;
  dmnsn_color color;
  dmnsn_CIE_xyY xyY;
  dmnsn_CIE_Lab Lab;
  dmnsn_CIE_Luv Luv;
  dmnsn_sRGB sRGB;
  FILE *ofile, *ifile;
  unsigned int i, j;
  const unsigned int x = 333, y = 300;

  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  /* Allocate a canvas */
  canvas = dmnsn_new_canvas(3*x, y);
  if (!canvas) {
    fprintf(stderr, "--- Allocation of canvas failed! ---\n");
    return EXIT_FAILURE;
  }

  /* Optimize the canvas for PNG export */
  if (dmnsn_png_optimize_canvas(canvas) != 0) {
    dmnsn_delete_canvas(canvas);
    fprintf(stderr, "--- Couldn't optimize canvas for PNG! ---\n");
    return EXIT_FAILURE;
  }

  for (i = 0; i < x; ++i) {
    for (j = 0; j < y; ++j) {
      /*
       * CIE xyY colorspace
       */
      xyY.Y = 0.5;
      xyY.x = ((double)i)/(x - 1);
      xyY.y = ((double)j)/(y - 1);

      color = dmnsn_color_from_xyY(xyY);
      sRGB  = dmnsn_sRGB_from_color(color);

      if (sRGB.R > 1.0 || sRGB.G > 1.0 || sRGB.B > 1.0
          || sRGB.R < 0.0 || sRGB.G < 0.0 || sRGB.B < 0.0) {
        /* Out of sRGB gamut */
        color.trans = 0.5;
      }

      dmnsn_set_pixel(canvas, i, j, color);

      /*
       * CIE Lab colorspace
       */
      Lab.L = 75.0;
      Lab.a = 200.0*(((double)i)/(x - 1) - 0.5);
      Lab.b = 200.0*(((double)j)/(y - 1) - 0.5);

      color = dmnsn_color_from_Lab(Lab, dmnsn_whitepoint);
      sRGB  = dmnsn_sRGB_from_color(color);

      if (sRGB.R > 1.0 || sRGB.G > 1.0 || sRGB.B > 1.0
          || sRGB.R < 0.0 || sRGB.G < 0.0 || sRGB.B < 0.0) {
        /* Out of sRGB gamut */
        color.trans = 0.5;
      }

      dmnsn_set_pixel(canvas, i + x, j, color);

      /*
       * CIE Luv colorspace
       */
      Luv.L = 75.0;
      Luv.u = 200.0*(((double)i)/(x - 1) - 0.5);
      Luv.v = 200.0*(((double)j)/(y - 1) - 0.5);

      color = dmnsn_color_from_Luv(Luv, dmnsn_whitepoint);
      sRGB  = dmnsn_sRGB_from_color(color);

      if (sRGB.R > 1.0 || sRGB.G > 1.0 || sRGB.B > 1.0
          || sRGB.R < 0.0 || sRGB.G < 0.0 || sRGB.B < 0.0) {
        /* Out of sRGB gamut */
        color.trans = 0.5;
      }

      dmnsn_set_pixel(canvas, i + 2*x, j, color);
    }
  }

  /* Write the image to PNG */

  ofile = fopen("dimension1.png", "wb");
  if (!ofile) {
    fprintf(stderr, "--- Opening 'dimension1.png' for writing failed! ---\n");
    dmnsn_delete_canvas(canvas);
    return EXIT_FAILURE;
  }

  progress = dmnsn_png_write_canvas_async(canvas, ofile);
  if (!progress) {
    fprintf(stderr, "--- Creating PNG writing worker thread failed! ---\n");
    fclose(ofile);
    dmnsn_delete_canvas(canvas);
    return EXIT_FAILURE;
  }

  progressbar("Writing PNG file: ", progress);

  if (dmnsn_finish_progress(progress) != 0) {
    fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
    fclose(ofile);
    dmnsn_delete_canvas(canvas);
    return EXIT_FAILURE;
  }

  fclose(ofile);
  dmnsn_delete_canvas(canvas);

  /* Read the image back from the PNG file */

  ifile = fopen("dimension1.png", "rb");
  if (!ifile) {
    fprintf(stderr, "--- Opening 'dimension1.png' for reading failed! ---\n");
    return EXIT_FAILURE;
  }

  progress = dmnsn_png_read_canvas_async(&canvas, ifile);
  if (!progress) {
    fprintf(stderr, "--- Creating PNG reading worker thread failed! ---\n");
    fclose(ifile);
    return EXIT_FAILURE;
  }

  progressbar("Reading PNG file: ", progress);

  if (dmnsn_finish_progress(progress) != 0) {
    fprintf(stderr, "--- Reading canvas from PNG failed! ---\n");
    fclose(ifile);
    return EXIT_FAILURE;
  }

  fclose(ifile);

  /* ... and write it back again */

  ofile = fopen("dimension2.png", "wb");
  if (!ofile) {
    fprintf(stderr, "--- Opening 'dimension2.png' for writing failed! ---\n");
    return EXIT_FAILURE;
  }

  progress = dmnsn_png_write_canvas_async(canvas, ofile);
  if (!progress) {
    fprintf(stderr, "--- Creating PNG writing worker thread failed! ---\n");
    fclose(ofile);
    dmnsn_delete_canvas(canvas);
    return EXIT_FAILURE;
  }

  progressbar("Writing PNG file: ", progress);

  if (dmnsn_finish_progress(progress) != 0) {
    fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
    fclose(ofile);
    dmnsn_delete_canvas(canvas);
    return EXIT_FAILURE;
  }

  fclose(ofile);
  dmnsn_delete_canvas(canvas);
  return EXIT_SUCCESS;
}
