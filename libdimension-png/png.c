/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Library.                           *
 *                                                                       *
 * The Dimension Library is free software; you can redistribute it and/  *
 * or modify it under the terms of the GNU Lesser General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Library is distributed in the hope that it will be      *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include "dimension-png.h"
#include <png.h>
#include <setjmp.h>
#include <arpa/inet.h>
#include <stdlib.h>

int
dmnsn_png_write_canvas(const dmnsn_canvas *canvas, FILE *file)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 width, height;
  unsigned int x, y;
  uint16_t *row = NULL;
  dmnsn_color color;
  dmnsn_sRGB sRGB;

  width = canvas->x;
  height = canvas->y;

  if (!file) return 1;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) return 1;

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, NULL);
    return 1;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    if (row) free(row);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 1;
  }

  png_init_io(png_ptr, file);

  png_set_IHDR(png_ptr, info_ptr, width, height, 16,
               PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_set_sRGB_gAMA_and_cHRM(png_ptr, info_ptr, PNG_sRGB_INTENT_ABSOLUTE);

  png_set_invert_alpha(png_ptr);
  png_write_info(png_ptr, info_ptr);

  if (htonl(1) != 1) {
    /* We are little-endian */
    png_set_swap(png_ptr);
  }

  row = malloc(4*sizeof(uint16_t)*width);
  if (!row) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 1;
  }

  for (y = 0; y < height; ++y) {
    for (x = 0; x < width; ++x) {
      /* Invert the rows.  PNG coordinates are fourth quadrant. */
      color = dmnsn_get_pixel(canvas, x, height - y - 1);
      sRGB = dmnsn_sRGB_from_color(color);

      if (sRGB.R <= 0.0) {
        row[4*x] = 0;
      } else if (sRGB.R >= 1.0) {
        row[4*x] = UINT16_MAX;
      } else {
        row[4*x] = sRGB.R*UINT16_MAX;
      }

      if (sRGB.G <= 0.0) {
        row[4*x + 1] = 0;
      } else if (sRGB.G >= 1.0) {
        row[4*x + 1] = UINT16_MAX;
      } else {
        row[4*x + 1] = sRGB.G*UINT16_MAX;
      }

      if (sRGB.B <= 0.0) {
        row[4*x + 2] = 0;
      } else if (sRGB.B >= 1.0) {
        row[4*x + 2] = UINT16_MAX;
      } else {
        row[4*x + 2] = sRGB.B*UINT16_MAX;
      }

      row[4*x + 3] = (color.filter + color.trans)*UINT16_MAX;
    }
    png_write_row(png_ptr, (png_bytep)row);
  }

  png_write_end(png_ptr, info_ptr);

  free(row);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  return 0;
}

dmnsn_canvas *
dmnsn_png_read_canvas(FILE *file)
{
  dmnsn_canvas *canvas;
  png_byte header[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 width, height, rowbytes;
  int bit_depth, color_type, interlace_type, compression_type, filter_method;
  png_bytep image = NULL;
  png_bytep *row_pointers = NULL;
  unsigned int x, y;
  dmnsn_color color;
  dmnsn_sRGB sRGB;
  png_bytep png_pixel;

  if (!file) return NULL;

  fread(header, 1, 8, file);
  if (png_sig_cmp(header, 0, 8)) return NULL;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) return NULL;

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return NULL;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    if (row_pointers) free(row_pointers);
    if (image) free(image);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  png_init_io(png_ptr, file);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, &compression_type, &filter_method);

  /*
   * - Convert paletted images to RGB.
   * - Convert a tRNS chunk to an alpha channel
   * - Convert grayscale to RGB
   * - Invert the alpha channel
   */
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_ptr);
  }
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png_ptr);
  }
  if (color_type == PNG_COLOR_TYPE_GRAY
      || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
          png_set_gray_to_rgb(png_ptr);
  }
  png_set_invert_alpha(png_ptr);

  png_read_update_info(png_ptr, info_ptr);
  rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  image = malloc(rowbytes*height);
  if (!image) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  row_pointers = malloc(sizeof(png_bytep)*height);
  if (!row_pointers) {
    free(image);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  for (y = 0; y < height; ++y) {
    row_pointers[y] = image + y*rowbytes;
  }

  png_read_image(png_ptr, row_pointers);

  canvas = dmnsn_new_canvas(width, height);
  if (!canvas) {
    free(row_pointers);
    free(image);
    png_read_end(png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  if (bit_depth == 16) {
    if (color_type & PNG_COLOR_MASK_ALPHA) {
      for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
          png_pixel = image + 8*(y*width + x);

          sRGB.R = ((double)((png_pixel[0] << UINT16_C(8)) + png_pixel[1]))
            /UINT16_MAX;
          sRGB.G = ((double)((png_pixel[2] << UINT16_C(8)) + png_pixel[3]))
            /UINT16_MAX;
          sRGB.B = ((double)((png_pixel[4] << UINT16_C(8)) + png_pixel[5]))
            /UINT16_MAX;

          color       = dmnsn_color_from_sRGB(sRGB);
          color.trans = ((double)((png_pixel[6] << UINT16_C(8))
                                  + png_pixel[7]))/UINT16_MAX;
          dmnsn_set_pixel(canvas, x, height - y - 1, color);
        }
      }
    } else {
      for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
          png_pixel = image + 6*(y*width + x);

          sRGB.R = ((double)((png_pixel[0] << UINT16_C(8)) + png_pixel[1]))
            /UINT16_MAX;
          sRGB.G = ((double)((png_pixel[2] << UINT16_C(8)) + png_pixel[3]))
            /UINT16_MAX;
          sRGB.B = ((double)((png_pixel[4] << UINT16_C(8)) + png_pixel[5]))
            /UINT16_MAX;

          color = dmnsn_color_from_sRGB(sRGB);
          dmnsn_set_pixel(canvas, x, height - y - 1, color);
        }
      }
    }
  } else {
    /* Bit depth is 8 */
    if (color_type & PNG_COLOR_MASK_ALPHA) {
      for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
          png_pixel = image + 4*(y*width + x);

          sRGB.R = ((double)png_pixel[0])/UINT8_MAX;
          sRGB.G = ((double)png_pixel[1])/UINT8_MAX;
          sRGB.B = ((double)png_pixel[2])/UINT8_MAX;

          color       = dmnsn_color_from_sRGB(sRGB);
          color.trans = ((double)png_pixel[3])/UINT8_MAX;
          dmnsn_set_pixel(canvas, x, height - y - 1, color);
        }
      }
    } else {
      for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
          png_pixel = image + 3*(y*width + x);

          sRGB.R = ((double)png_pixel[0])/UINT8_MAX;
          sRGB.G = ((double)png_pixel[1])/UINT8_MAX;
          sRGB.B = ((double)png_pixel[2])/UINT8_MAX;

          color = dmnsn_color_from_sRGB(sRGB);
          dmnsn_set_pixel(canvas, x, height - y - 1, color);
        }
      }
    }
  }

  free(row_pointers);
  free(image);
  png_read_end(png_ptr, NULL);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  return canvas;
}
