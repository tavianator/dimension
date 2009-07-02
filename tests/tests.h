/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
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
#include <GL/glx.h>
#include <GL/gl.h>
#include <stdio.h>

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

/* Create a new X window */
dmnsn_display *dmnsn_new_X_display(const dmnsn_canvas *canvas);
dmnsn_display *dmnsn_new_glX_display(const dmnsn_canvas *canvas);
/* Destroy the X window */
void dmnsn_delete_display(dmnsn_display *display);

/* Flush the GL buffers */
void dmnsn_display_frame(dmnsn_display *display);

/*
 * Asynchronicity
 */

/* Print a progress bar of the progress of `progress' */
void progressbar(const char *str, const dmnsn_progress *progress);
