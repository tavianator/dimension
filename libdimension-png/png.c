/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of Dimension.                                       *
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
  dmnsn_pixel *pixel;

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
      pixel = canvas->pixels + (height - y - 1)*width + x;

      row[4*x] = pixel->r;
      row[4*x + 1] = pixel->g;
      row[4*x + 2] = pixel->b;

      if (pixel->a > pixel->t) {
        row[4*x + 3] = pixel->a;
      } else {
        row[4*x + 3] = pixel->t;
      }
    }
    png_write_row(png_ptr, (png_bytep)row);
  }

  png_write_end(png_ptr, info_ptr);

  free(row);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  return 0;
}

int
dmnsn_png_read_canvas(dmnsn_canvas **canvas, FILE *file)
{
  png_byte header[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 width, height, rowbytes;
  int bit_depth, color_type, interlace_type, compression_type, filter_method;
  png_bytep image = NULL;
  png_bytep *row_pointers = NULL;
  unsigned int x, y;
  dmnsn_pixel *pixel;
  png_bytep png_pixel;

  /* Ensure that *canvas can always be deleted. */
  *canvas = NULL;

  if (!file) return 1;

  fread(header, 1, 8, file);
  if (png_sig_cmp(header, 0, 8)) return 1;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) return 1;

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return 1;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    if (row_pointers) free(row_pointers);
    if (image) free(image);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 1;
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
    return 1;
  }

  row_pointers = malloc(sizeof(png_bytep)*height);
  if (!row_pointers) {
    free(image);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 1;
  }

  for (y = 0; y < height; ++y) {
    row_pointers[y] = image + y*rowbytes;
  }

  png_read_image(png_ptr, row_pointers);

  *canvas = dmnsn_new_canvas(width, height);
  if (!*canvas) {
    free(row_pointers);
    free(image);
    png_read_end(png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 1;
  }

  if (bit_depth == 16) {
    if (color_type & PNG_COLOR_MASK_ALPHA) {
      for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
          pixel = (*canvas)->pixels + (height - y - 1)*width + x;
          png_pixel = image + 8*(y*width + x);

          pixel->r = (png_pixel[0] << UINT16_C(8)) + png_pixel[1];
          pixel->g = (png_pixel[2] << UINT16_C(8)) + png_pixel[3];
          pixel->b = (png_pixel[4] << UINT16_C(8)) + png_pixel[5];
          pixel->a = (png_pixel[6] << UINT16_C(8)) + png_pixel[7];
          pixel->t = 0;
        }
      }
    } else {
      for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
          pixel = (*canvas)->pixels + (height - y - 1)*width + x;
          png_pixel = image + 6*(y*width + x);

          pixel->r = (png_pixel[0] << UINT16_C(8)) + png_pixel[1];
          pixel->g = (png_pixel[2] << UINT16_C(8)) + png_pixel[3];
          pixel->b = (png_pixel[4] << UINT16_C(8)) + png_pixel[5];
          pixel->a = 0;
          pixel->t = 0;
        }
      }
    }
  } else {
    /* Bit depth is 8 */
    if (color_type & PNG_COLOR_MASK_ALPHA) {
      for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
          pixel = (*canvas)->pixels + (height - y - 1)*width + x;
          png_pixel = image + 4*(y*width + x);

          pixel->r = png_pixel[0] << UINT16_C(8);
          pixel->g = png_pixel[1] << UINT16_C(8);
          pixel->b = png_pixel[2] << UINT16_C(8);
          pixel->a = png_pixel[3] << UINT16_C(8);
          pixel->t = 0;
        }
      }
    } else {
      for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
          pixel = (*canvas)->pixels + (height - y - 1)*width + x;
          png_pixel = image + 3*(y*width + x);

          pixel->r = png_pixel[0] << UINT16_C(8);
          pixel->g = png_pixel[1] << UINT16_C(8);
          pixel->b = png_pixel[2] << UINT16_C(8);
          pixel->a = 0;
          pixel->t = 0;
        }
      }
    }
  }

  free(row_pointers);
  free(image);
  png_read_end(png_ptr, NULL);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  return 0;
}
