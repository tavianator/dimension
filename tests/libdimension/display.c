/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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
#include <GL/glx.h>
#include <GL/gl.h>
#include <stdlib.h>

struct dmnsn_display {
  Display *dpy;
  Window win;
  Colormap cmap;
  GLXContext cx;
  XEvent event;
  XVisualInfo *vi;
};

/* XIfEvent callback */
static Bool
WaitForNotify(Display *d, XEvent *e, char *arg)
{
  return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}

dmnsn_display *
dmnsn_new_display(const dmnsn_canvas *canvas)
{
  int attributeList[] = {
    GLX_RGBA,
    GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 1,
    GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1,
    None
  };
  XSetWindowAttributes swa;
  dmnsn_display *display;

  display = dmnsn_malloc(sizeof(dmnsn_display));

  display->dpy  = NULL;
  display->win  = 0;
  display->cmap = 0;
  display->cx   = NULL;
  display->vi   = NULL;

  /* Get an X connection */
  display->dpy = XOpenDisplay(0);
  if (!display->dpy) {
    dmnsn_delete_display(display);
    return NULL;
  }

  /* Get an appropriate visual */
  display->vi = glXChooseVisual(display->dpy, DefaultScreen(display->dpy),
                                attributeList);
  if (!display->vi) {
    dmnsn_delete_display(display);
    return NULL;
  }

  /* Create a GLX context */
  display->cx = glXCreateContext(display->dpy, display->vi, 0, GL_TRUE);
  if (!display->cx) {
    dmnsn_delete_display(display);
    return NULL;
  }

  /* Create a color map */
  display->cmap = XCreateColormap(display->dpy,
                                  RootWindow(display->dpy, display->vi->screen),
                                  display->vi->visual, AllocNone);
  if (!display->cmap) {
    dmnsn_delete_display(display);
    return NULL;
  }

  /* Create a window */
  swa.colormap = display->cmap;
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask;
  display->win = XCreateWindow(display->dpy,
                               RootWindow(display->dpy, display->vi->screen),
                               0, 0, canvas->width, canvas->height,
                               0, display->vi->depth, InputOutput,
                               display->vi->visual,
                               CWBorderPixel|CWColormap|CWEventMask, &swa);
  if (!display->win) {
    dmnsn_delete_display(display);
    return NULL;
  }

  XStoreName(display->dpy, display->win, "glX");

  XMapWindow(display->dpy, display->win);
  XIfEvent(display->dpy, &display->event, WaitForNotify, (char*)display->win);

  /* Connect the context to the window */
  glXMakeCurrent(display->dpy, display->win, display->cx);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  dmnsn_display_flush(display);

  return display;
}

void
dmnsn_delete_display(dmnsn_display *display)
{
  if (display) {
    if (display->win)
      XDestroyWindow(display->dpy, display->win);

    if (display->cmap)
      XFreeColormap(display->dpy, display->cmap);

    if (display->cx)
      glXDestroyContext(display->dpy, display->cx);

    if (display->vi)
      XFree(display->vi);

    if (display->dpy)
      XCloseDisplay(display->dpy);

    dmnsn_free(display);
  }
}

void
dmnsn_display_flush(dmnsn_display *display)
{
  glXWaitX();
  glXSwapBuffers(display->dpy, display->win);
}
