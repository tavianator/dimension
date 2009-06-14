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

#include "dimension.h"
#include <png.h>
#include <setjmp.h>
#include <arpa/inet.h>
#include <stdlib.h>

/* Write a canvas to a png file, using libpng.  Return 0 on success, nonzero on
   failure. */
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

  if (!file) return 1; /* file was NULL */

  width = canvas->x;
  height = canvas->y;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) return 1; /* Couldn't create libpng write struct */

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    /* Couldn't create libpng info struct */
    png_destroy_write_struct(&png_ptr, NULL);
    return 1;
  }

  /* libpng will longjmp here if it encounters an error from here on */
  if (setjmp(png_jmpbuf(png_ptr))) {
    /* libpng error */
    if (row) free(row);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 1;
  }

  /* Associate file with the libpng write struct */
  png_init_io(png_ptr, file);

  /* Set header correctly for 16-bit sRGB image */
  png_set_IHDR(png_ptr, info_ptr, width, height, 16,
               PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_sRGB_gAMA_and_cHRM(png_ptr, info_ptr, PNG_sRGB_INTENT_ABSOLUTE);

  /* We think of transparency in the opposite way that PNG does */
  png_set_invert_alpha(png_ptr);

  /* Write the info struct */
  png_write_info(png_ptr, info_ptr);

  if (htonl(1) != 1) {
    /* We are little-endian; swap the byte order of the pixels */
    png_set_swap(png_ptr);
  }

  /* Allocate the temporary row of RGBA values */
  row = malloc(4*sizeof(uint16_t)*width);
  if (!row) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 1;
  }

  /* Write the pixels */
  for (y = 0; y < height; ++y) {
    for (x = 0; x < width; ++x) {
      /* Invert the rows.  PNG coordinates are fourth quadrant. */
      color = dmnsn_get_pixel(canvas, x, height - y - 1);
      sRGB = dmnsn_sRGB_from_color(color);

      /* Saturate R, G, and B to [0, UINT16_MAX] */

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

      /* color.filter + color.trans is in [0.0, 1.0] by definition */
      row[4*x + 3] = (color.filter + color.trans)*UINT16_MAX;
    }

    /* Write the row */
    png_write_row(png_ptr, (png_bytep)row);
  }

  /* Finish the PNG file */
  png_write_end(png_ptr, info_ptr);

  free(row);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  return 0;
}

/* Read a canvas from the PNG file `file'.  Return NULL on error. */
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

  if (!file) return NULL; /* file was NULL */

  fread(header, 1, 8, file);
  if (png_sig_cmp(header, 0, 8)) return NULL; /* file is not a PNG file */

  /* Create the libpng read struct */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) return NULL;

  /* Create the libpng info struct */
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return NULL;
  }

  /* libpng will longjmp here if it encounters an error from here on */
  if (setjmp(png_jmpbuf(png_ptr))) {
    /* libpng error */
    if (row_pointers) free(row_pointers);
    if (image) free(image);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  /* Associate the read struct with the file, and tell it we've already checked
     8 bytes of signature */
  png_init_io(png_ptr, file);
  png_set_sig_bytes(png_ptr, 8);

  /* Read the PNG header into info struct */
  png_read_info(png_ptr, info_ptr);

  /* Get useful information from the info struct */
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

  /* Update the info struct */
  png_read_update_info(png_ptr, info_ptr);

  /* Get bytes/image row */
  rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  /* Allocate the temporary image buffer */
  image = malloc(rowbytes*height);
  if (!image) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  /* Allocate and set an array of pointers to rows in image */

  row_pointers = malloc(sizeof(png_bytep)*height);
  if (!row_pointers) {
    free(image);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  for (y = 0; y < height; ++y) {
    row_pointers[y] = image + y*rowbytes;
  }

  /* Read the image to image all at once.  At the expense of greater memory use,
     this handles interlacing for us. */
  png_read_image(png_ptr, row_pointers);

  /* Allocate the canvas */
  canvas = dmnsn_new_canvas(width, height);
  if (!canvas) {
    free(row_pointers);
    free(image);
    png_read_end(png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  /* Now we convert the image to our canvas format.  This depends on the image
     bit depth (which has been scaled up to at least 8 or 16), and the presence
     of an alpha channel.  For performance reasons, the tests are outside the
     loops, although that doesn't really matter for a decent compiler. */
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