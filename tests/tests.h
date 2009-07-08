/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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

#ifndef TESTS_H
#define TESTS_H

#include "../libdimension/dimension.h"
#include <GL/glx.h>
#include <GL/gl.h>
#include <stdio.h>

#ifdef __cplusplus
/* We've been included from a C++ file; mark everything here as extern "C" */
extern "C" {
#endif

/*
 * Convenience
 */

dmnsn_scene *dmnsn_new_default_scene();
void dmnsn_delete_default_scene(dmnsn_scene *scene);

/*
 * Windowing
 */

typedef struct {
  Display *dpy;
  Window win;
  Colormap cmap;
  GLXContext cx;
  XEvent event;
} dmnsn_display;

dmnsn_display *dmnsn_new_display(const dmnsn_canvas *canvas);
void dmnsn_delete_display(dmnsn_display *display);

/* Flush the GL buffers */
void dmnsn_display_frame(dmnsn_display *display);

/*
 * Asynchronicity
 */

/* Print a progress bar of the progress of `progress' */
void progressbar(const char *str, const dmnsn_progress *progress);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_H */
