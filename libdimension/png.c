/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

/**
 * @file
 * PNG import/export.
 */

#include "dimension-internal.h"
#include <png.h>
#include <errno.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdint.h>

/* Optimize canvas for PNG exporting */
int
dmnsn_png_optimize_canvas(dmnsn_canvas *canvas)
{
  dmnsn_rgba16_optimize_canvas(canvas);
  return 0;
}

/** Payload type for PNG write thread callback. */
typedef struct {
  dmnsn_future *future;
  const dmnsn_canvas *canvas;
  FILE *file;
} dmnsn_png_write_payload;

/** Payload type for PNG read thread callback. */
typedef struct {
  dmnsn_future *future;
  dmnsn_canvas **canvas;
  dmnsn_pool *pool;
  FILE *file;
} dmnsn_png_read_payload;

/** PNG write thread callback. */
static int dmnsn_png_write_canvas_thread(void *ptr);
/** PNG read thread callback. */
static int dmnsn_png_read_canvas_thread(void *ptr);

/* Write a canvas to a png file, using libpng.  Return 0 on success, nonzero on
   failure. */
int
dmnsn_png_write_canvas(const dmnsn_canvas *canvas, FILE *file)
{
  dmnsn_future *future = dmnsn_png_write_canvas_async(canvas, file);
  return dmnsn_future_join(future);
}

/* Write a canvas to a png file in the background */
dmnsn_future *
dmnsn_png_write_canvas_async(const dmnsn_canvas *canvas, FILE *file)
{
  dmnsn_future *future = dmnsn_new_future();

  dmnsn_png_write_payload *payload = DMNSN_MALLOC(dmnsn_png_write_payload);
  payload->future = future;
  payload->canvas = canvas;
  payload->file   = file;

  /* Create the worker thread */
  dmnsn_new_thread(future, dmnsn_png_write_canvas_thread, payload);

  return future;
}

/* Read a canvas from the PNG file `file'.  Return NULL on error. */
dmnsn_canvas *
dmnsn_png_read_canvas(dmnsn_pool *pool, FILE *file)
{
  dmnsn_canvas *canvas;
  dmnsn_future *future = dmnsn_png_read_canvas_async(&canvas, pool, file);
  dmnsn_future_join(future);
  return canvas;
}

/* Read a canvas from a png file in the background */
dmnsn_future *
dmnsn_png_read_canvas_async(dmnsn_canvas **canvas, dmnsn_pool *pool, FILE *file)
{
  dmnsn_future *future = dmnsn_new_future();
  dmnsn_png_read_payload *payload = DMNSN_MALLOC(dmnsn_png_read_payload);

  payload->future = future;
  payload->canvas = canvas;
  *payload->canvas = NULL;
  payload->pool = pool;
  payload->file = file;

  /* Create the worker thread */
  dmnsn_new_thread(future, dmnsn_png_read_canvas_thread, payload);

  return future;
}

/*
 * Thread callbacks
 */

/* Write a PNG file */
static int
dmnsn_png_write_canvas_thread(void *ptr)
{
  dmnsn_png_write_payload *payload = ptr;

  png_uint_32 width = payload->canvas->width;
  png_uint_32 height = payload->canvas->height;

  dmnsn_future_set_total(payload->future, height);

  png_structp png_ptr
    = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    /* Couldn't create libpng write struct */
    dmnsn_free(payload);
    return -1;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    /* Couldn't create libpng info struct */
    png_destroy_write_struct(&png_ptr, NULL);
    dmnsn_free(payload);
    return -1;
  }

  /* libpng will longjmp here if it encounters an error from here on */
  uint16_t *row = NULL;
  if (setjmp(png_jmpbuf(png_ptr))) {
    /* libpng error */
    dmnsn_free(row);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    dmnsn_free(payload);
    return -1;
  }

  /* Associate file with the libpng write struct */
  png_init_io(png_ptr, payload->file);

  /* Set header correctly for 16-bit sRGB image */
  png_set_IHDR(png_ptr, info_ptr, width, height, 16,
               PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_sRGB_gAMA_and_cHRM(png_ptr, info_ptr, PNG_sRGB_INTENT_ABSOLUTE);

  /* We think of transparency in the opposite way that PNG does */
  png_set_invert_alpha(png_ptr);

  /* Write the info struct */
  png_write_info(png_ptr, info_ptr);

  if (dmnsn_is_little_endian()) {
    /* We are little-endian; swap the byte order of the pixels */
    png_set_swap(png_ptr);
  }

  /* Check if we can optimize this */
  dmnsn_canvas_optimizer *optimizer =
    dmnsn_canvas_find_optimizer(payload->canvas, dmnsn_rgba16_optimizer_fn);
  if (optimizer) {
    for (size_t y = 0; y < height; ++y) {
      /* Invert the rows.  PNG coordinates are fourth quadrant. */
      uint16_t *row_opt = (uint16_t *)optimizer->ptr + 4*(height - y - 1)*width;
      png_write_row(png_ptr, (png_bytep)row_opt);
      dmnsn_future_increment(payload->future);
    }

    /* Finish the PNG file */
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    dmnsn_free(payload);
    return 0;
  }

  /* Allocate the temporary row of RGBA values */
  row = dmnsn_malloc(4*sizeof(uint16_t)*width);

  /* Write the pixels */
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      /* Invert the rows.  PNG coordinates are fourth quadrant. */
      dmnsn_tcolor tcolor = dmnsn_canvas_get_pixel(payload->canvas,
                                                   x, height - y - 1);
      tcolor = dmnsn_tcolor_remove_filter(tcolor);
      tcolor.c = dmnsn_color_to_sRGB(tcolor.c);
      tcolor = dmnsn_tcolor_saturate(tcolor);

      row[4*x]     = lround(tcolor.c.R*UINT16_MAX);
      row[4*x + 1] = lround(tcolor.c.G*UINT16_MAX);
      row[4*x + 2] = lround(tcolor.c.B*UINT16_MAX);
      row[4*x + 3] = lround(tcolor.T*UINT16_MAX);
    }

    /* Write the row */
    png_write_row(png_ptr, (png_bytep)row);
    dmnsn_future_increment(payload->future);
  }

  /* Finish the PNG file */
  png_write_end(png_ptr, info_ptr);

  dmnsn_free(row);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  dmnsn_free(payload);
  return 0;
}

/** Thread-specific pointer to the appropriate dmnsn_future* for
    dmnsn_png_read_row_callback. */
static __thread dmnsn_future *dmnsn_tl_png_read_future;

/** Callback to increment the progress after a row has been read. */
static void
dmnsn_png_read_row_callback(png_structp png_ptr, png_uint_32 row, int pass)
{
  dmnsn_future_increment(dmnsn_tl_png_read_future);
}

/* Read a PNG file */
static int
dmnsn_png_read_canvas_thread(void *ptr)
{
  dmnsn_png_read_payload *payload = ptr;
  dmnsn_tl_png_read_future = payload->future;

  png_byte header[8];
  if (fread(header, 1, 8, payload->file) != 8) {
    dmnsn_free(payload);
    return -1;
  }
  if (png_sig_cmp(header, 0, 8)) {
    /* payload->file is not a PNG file */
    dmnsn_free(payload);
    errno = EINVAL;
    return -1;
  }

  /* Create the libpng read struct */
  png_structp png_ptr
    = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    dmnsn_free(payload);
    return -1;
  }

  /* Create the libpng info struct */
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    dmnsn_free(payload);
    return -1;
  }

  /* libpng will longjmp here if it encounters an error from here on */
  png_bytep image = NULL;
  png_bytep *row_pointers = NULL;
  if (setjmp(png_jmpbuf(png_ptr))) {
    /* libpng error */
    dmnsn_free(row_pointers);
    dmnsn_free(image);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    dmnsn_free(payload);
    return -1;
  }

  /* Associate the read struct with the file, and tell it we've already checked
     8 bytes of signature */
  png_init_io(png_ptr, payload->file);
  png_set_sig_bytes(png_ptr, 8);

  /* Read the PNG header into info struct */
  png_read_info(png_ptr, info_ptr);

  /* Get useful information from the info struct */
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type, compression_type, filter_method;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, &compression_type, &filter_method);
  int number_of_passes = png_set_interlace_handling(png_ptr);

  dmnsn_future_set_total(payload->future, (number_of_passes + 1)*height);
  png_set_read_status_fn(png_ptr, dmnsn_png_read_row_callback);

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
      || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
  {
    png_set_gray_to_rgb(png_ptr);
  }
  png_set_invert_alpha(png_ptr);

  /* Update the info struct */
  png_read_update_info(png_ptr, info_ptr);

  /* Get bytes/image row */
  png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  /* Allocate the temporary image buffer */
  image = dmnsn_malloc(rowbytes*height);

  /* Allocate and set an array of pointers to rows in image */
  row_pointers = dmnsn_malloc(sizeof(png_bytep)*height);

  for (size_t y = 0; y < height; ++y) {
    row_pointers[y] = image + y*rowbytes;
  }

  /* Read the image to memory all at once.  At the expense of greater memory
     use, this handles interlacing for us. */
  png_read_image(png_ptr, row_pointers);

  /* Allocate the canvas */
  *payload->canvas = dmnsn_new_canvas(payload->pool, width, height);

  /* Now we convert the image to our canvas format.  This depends on the image
     bit depth (which has been scaled up to at least 8 or 16), and the presence
     of an alpha channel. */
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      dmnsn_tcolor tcolor;
      tcolor.F = 0.0;

      if (color_type & PNG_COLOR_MASK_ALPHA) {
        if (bit_depth == 16) {
          png_bytep png_pixel = image + 8*(y*width + x);
          tcolor.c.R = (double)((png_pixel[0] << 8) + png_pixel[1])/UINT16_MAX;
          tcolor.c.G = (double)((png_pixel[2] << 8) + png_pixel[3])/UINT16_MAX;
          tcolor.c.B = (double)((png_pixel[4] << 8) + png_pixel[5])/UINT16_MAX;
          tcolor.T   = (double)((png_pixel[6] << 8) + png_pixel[7])/UINT16_MAX;
        } else {
          png_bytep png_pixel = image + 4*(y*width + x);
          tcolor.c.R = (double)png_pixel[0]/UINT8_MAX;
          tcolor.c.G = (double)png_pixel[1]/UINT8_MAX;
          tcolor.c.B = (double)png_pixel[2]/UINT8_MAX;
          tcolor.T   = (double)png_pixel[3]/UINT8_MAX;
        }
      } else {
        tcolor.T = 0.0;

        if (bit_depth == 16) {
          png_bytep png_pixel = image + 6*(y*width + x);
          tcolor.c.R = (double)((png_pixel[0] << 8) + png_pixel[1])/UINT16_MAX;
          tcolor.c.G = (double)((png_pixel[2] << 8) + png_pixel[3])/UINT16_MAX;
          tcolor.c.B = (double)((png_pixel[4] << 8) + png_pixel[5])/UINT16_MAX;
        } else {
          png_bytep png_pixel = image + 3*(y*width + x);
          tcolor.c.R = (double)png_pixel[0]/UINT8_MAX;
          tcolor.c.G = (double)png_pixel[1]/UINT8_MAX;
          tcolor.c.B = (double)png_pixel[2]/UINT8_MAX;
        }
      }

      tcolor.c = dmnsn_color_from_sRGB(tcolor.c);
      dmnsn_canvas_set_pixel(*payload->canvas, x, height - y - 1, tcolor);
    }

    dmnsn_future_increment(payload->future);
  }

  dmnsn_free(row_pointers);
  dmnsn_free(image);
  png_read_end(png_ptr, NULL);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  dmnsn_free(payload);
  return 0;
}
