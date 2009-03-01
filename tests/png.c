#include "../libdimension/dimension.h"
#include "../libdimension-png/dimension-png.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main() {
  dmnsn_canvas *canvas;
  FILE *ofile, *ifile;
  unsigned int x, y;

  canvas = dmnsn_new_canvas(333, 300);
  if (!canvas) {
    fprintf(stderr, "--- Allocation of canvas failed! ---\n");
    return EXIT_FAILURE;
  }

  ofile = fopen("dimension.png", "wb");
  if (!ofile) {
    fprintf(stderr, "--- Opening 'dimension.png' for writing failed! ---\n");
    return EXIT_FAILURE;
  }

  for (x = 0; x < canvas->x; ++x) {
    for (y = 0; y < canvas->y; ++y) {
      if (x < canvas->x/3) {
        canvas->pixels[y*canvas->x + x].r = UINT16_MAX;
        canvas->pixels[y*canvas->x + x].g = 0;
        canvas->pixels[y*canvas->x + x].b = 0;
      } else if (x < 2*canvas->x/3) {
        canvas->pixels[y*canvas->x + x].r = 0;
        canvas->pixels[y*canvas->x + x].g = UINT16_MAX;
        canvas->pixels[y*canvas->x + x].b = 0;
      } else {
        canvas->pixels[y*canvas->x + x].r = 0;
        canvas->pixels[y*canvas->x + x].g = 0;
        canvas->pixels[y*canvas->x + x].b = UINT16_MAX;
      }

      if (y < canvas->y/3) {
        canvas->pixels[y*canvas->x + x].a = 0;
        canvas->pixels[y*canvas->x + x].t = 0;
      } else if (y < 2*canvas->y/3) {
        canvas->pixels[y*canvas->x + x].a = UINT16_MAX/4;
        canvas->pixels[y*canvas->x + x].t = 0;
      } else {
        canvas->pixels[y*canvas->x + x].a = UINT16_MAX/4;
        canvas->pixels[y*canvas->x + x].t = UINT16_MAX/2;
      }
    }
  }

  if (dmnsn_png_write_canvas(canvas, ofile)) {
    fprintf(stderr, "--- Writing canvas failed! ---\n");
    return EXIT_FAILURE;
  }

  dmnsn_delete_canvas(canvas);

  fclose(ofile);
  ifile = fopen("dimension.png", "rb");
  if (!ifile) {
    fprintf(stderr, "--- Opening 'dimension.png' for reading failed! ---\n");
    return EXIT_FAILURE;
  }

  if (dmnsn_png_read_canvas(&canvas, ifile)) {
    fprintf(stderr, "--- Reading canvas failed! ---\n");
    return EXIT_FAILURE;
  }

  fclose(ifile);
  ofile = fopen("dimension.png", "wb");
  if (!ofile) {
    fprintf(stderr, "--- Opening 'dimension.png' for writing failed! ---\n");
    return EXIT_FAILURE;
  }

  if (dmnsn_png_write_canvas(canvas, ofile)) {
    fprintf(stderr, "--- Writing canvas failed! ---\n");
    return EXIT_FAILURE;
  }

  dmnsn_delete_canvas(canvas);
  return EXIT_SUCCESS;
}
