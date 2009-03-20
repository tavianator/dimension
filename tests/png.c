#include "../libdimension/dimension.h"
#include "../libdimension-png/dimension-png.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main() {
  dmnsn_canvas *canvas;
  dmnsn_sRGB sRGB;
  FILE *ofile, *ifile;
  unsigned int x, y;

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
        canvas->pixels[y*canvas->x + x] = dmnsn_color_from_sRGB(sRGB);
      } else if (x < 2*canvas->x/3) {
        sRGB.R = 0.0;
        sRGB.G = 1.0;
        sRGB.B = 0.0;
        canvas->pixels[y*canvas->x + x] = dmnsn_color_from_sRGB(sRGB);
      } else {
        sRGB.R = 0.0;
        sRGB.G = 0.0;
        sRGB.B = 1.0;
        canvas->pixels[y*canvas->x + x] = dmnsn_color_from_sRGB(sRGB);
      }

      if (y < canvas->y/3) {
        canvas->pixels[y*canvas->x + x].filter = 0.0;
        canvas->pixels[y*canvas->x + x].trans  = 0.0;
      } else if (y < 2*canvas->y/3) {
        canvas->pixels[y*canvas->x + x].filter = 1.0/3.0;
        canvas->pixels[y*canvas->x + x].trans  = 0.0;
      } else {
        canvas->pixels[y*canvas->x + x].filter = 1.0/3.0;
        canvas->pixels[y*canvas->x + x].trans  = 1.0/3.0;
      }
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
