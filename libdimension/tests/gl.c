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

int
main(void)
{
  /* Treat warnings as errors for tests */
  dmnsn_die_on_warnings(true);

  /* Allocate our canvas */
  dmnsn_pool *pool = dmnsn_new_pool();
  dmnsn_canvas *canvas = dmnsn_new_canvas(pool, 768, 480);

  /* Paint the test pattern */
  dmnsn_paint_test_canvas(canvas);

  /* Create a new glX display */
  dmnsn_display *display = dmnsn_new_display(canvas);
  if (!display) {
    fprintf(stderr, "--- WARNING: Couldn't initialize X or glX! ---\n");
    dmnsn_delete_pool(pool);
    return EXIT_SUCCESS;
  }

  /* Draw to OpenGL */
  printf("Drawing to OpenGL\n");
  if (dmnsn_gl_write_canvas(canvas) != 0) {
    dmnsn_delete_display(display);
    dmnsn_delete_pool(pool);
    fprintf(stderr, "--- Drawing to OpenGL failed! ---\n");
    return EXIT_FAILURE;
  }
  dmnsn_display_flush(display);

  /*
   * Now test GL import/export
   */

  /* Optimize the canvas for GL drawing */
  if (dmnsn_gl_optimize_canvas(canvas) != 0) {
    dmnsn_delete_pool(pool);
    fprintf(stderr, "--- Couldn't optimize canvas for GL! ---\n");
    return EXIT_FAILURE;
  }

  /* Read the image back from OpenGL */
  printf("Reading from OpenGL\n");
  if (dmnsn_gl_read_canvas(canvas, 0, 0) != 0) {
    dmnsn_delete_display(display);
    dmnsn_delete_pool(pool);
    fprintf(stderr, "--- Reading from OpenGL failed! ---\n");
    return EXIT_FAILURE;
  }

  /* And draw it back */
  printf("Drawing to OpenGL\n");
  if (dmnsn_gl_write_canvas(canvas) != 0) {
    dmnsn_delete_display(display);
    dmnsn_delete_pool(pool);
    fprintf(stderr, "--- Drawing to OpenGL failed! ---\n");
    return EXIT_FAILURE;
  }

  dmnsn_display_flush(display);
  dmnsn_delete_display(display);
  dmnsn_delete_pool(pool);
  return EXIT_SUCCESS;
}
