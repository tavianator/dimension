/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of the Dimension Test Suite.                        *
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

#include "../libdimension/dimension.h"
#include "../libdimension-png/dimension-png.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main() {
  dmnsn_canvas *canvas;
  dmnsn_sRGB sRGB;
  dmnsn_color color;
  FILE *ofile, *ifile;
  unsigned int x, y;

  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  canvas = dmnsn_new_canvas(333, 300);
  if (!canvas) {
    fprintf(stderr, "--- Allocation of canvas failed! ---\n");
    return EXIT_FAILURE;
  }

  ofile = fopen("dimension1.png", "wb");
  if (!ofile) {
    fprintf(stderr, "--- Opening 'dimension1.png' for writing failed! ---\n");
    return EXIT_FAILURE;
  }

  for (x = 0; x < canvas->x; ++x) {
    for (y = 0; y < canvas->y; ++y) {
      if (x < canvas->x/3) {
        sRGB.R = 1.0;
        sRGB.G = 0.0;
        sRGB.B = 0.0;
      } else if (x < 2*canvas->x/3) {
        sRGB.R = 0.0;
        sRGB.G = 1.0;
        sRGB.B = 0.0;
      } else {
        sRGB.R = 0.0;
        sRGB.G = 0.0;
        sRGB.B = 1.0;
      }
      color = dmnsn_color_from_sRGB(sRGB);

      if (y < canvas->y/3) {
        color.filter = 0.0;
        color.trans  = 0.0;
      } else if (y < 2*canvas->y/3) {
        color.filter = 1.0/3.0;
        color.trans  = 0.0;
      } else {
        color.filter = 1.0/3.0;
        color.trans  = 1.0/3.0;
      }

      dmnsn_set_pixel(canvas, color, x, y);
    }
  }

  if (dmnsn_png_write_canvas(canvas, ofile)) {
    fprintf(stderr, "--- Writing canvas failed! ---\n");
    return EXIT_FAILURE;
  }

  dmnsn_delete_canvas(canvas);

  fclose(ofile);
  ifile = fopen("dimension1.png", "rb");
  if (!ifile) {
    fprintf(stderr, "--- Opening 'dimension1.png' for reading failed! ---\n");
    return EXIT_FAILURE;
  }

  if (dmnsn_png_read_canvas(&canvas, ifile)) {
    fprintf(stderr, "--- Reading canvas failed! ---\n");
    return EXIT_FAILURE;
  }

  fclose(ifile);
  ofile = fopen("dimension2.png", "wb");
  if (!ofile) {
    fprintf(stderr, "--- Opening 'dimension2.png' for writing failed! ---\n");
    return EXIT_FAILURE;
  }

  if (dmnsn_png_write_canvas(canvas, ofile)) {
    fprintf(stderr, "--- Writing canvas failed! ---\n");
    return EXIT_FAILURE;
  }

  dmnsn_delete_canvas(canvas);
  return EXIT_SUCCESS;
}
