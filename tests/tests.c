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

#include "tests.h"

/* XIfEvent callback */
static Bool
WaitForNotify(Display *d, XEvent *e, char *arg)
{
  return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}

/* New non-glX X window */
dmnsn_display *
dmnsn_new_X_display(const dmnsn_canvas *canvas)
{
  dmnsn_display *display;
  int screen, depth;
  Visual *visual;
  XSetWindowAttributes swa;

  display = malloc(sizeof(dmnsn_display));
  if (!display) {
    return NULL;
  }

  /* Get an X connection */
  display->dpy = XOpenDisplay(0);
  if (!display->dpy) {
    free(display);
    return NULL;
  }
  screen = DefaultScreen(display->dpy);
  depth  = DefaultDepth(display->dpy, screen);

  /* Get an appropriate visual */
  visual = DefaultVisual(display->dpy, screen);

  /* Set display->cx to NULL */
  display->cx = NULL;

  /* Create a color map */
  display->cmap = XCreateColormap(display->dpy,
                                  RootWindow(display->dpy, screen),
                                  visual, AllocNone);
  if (!display->cmap) {
    glXDestroyContext(display->dpy, display->cx);
    XCloseDisplay(display->dpy);
    free(display);
    return NULL;
  }

  /* Create a window */
  swa.colormap = display->cmap;
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask;
  display->win = XCreateWindow(display->dpy,
                               RootWindow(display->dpy, screen),
                               0, 0, canvas->x, canvas->y,
                               0, depth, InputOutput, visual,
                               CWBorderPixel|CWColormap|CWEventMask, &swa);
  if (!display->win) {
    XFreeColormap(display->dpy, display->cmap);
    glXDestroyContext(display->dpy, display->cx);
    XCloseDisplay(display->dpy);
    free(display);
    return NULL;
  }

  XStoreName(display->dpy, display->win, "X");

  XMapWindow(display->dpy, display->win);
  XIfEvent(display->dpy, &display->event, WaitForNotify, (char*)display->win);

  return display;
}

dmnsn_display *
dmnsn_new_glX_display(const dmnsn_canvas *canvas)
{
  int attributeList[] = {
    GLX_RGBA,
    GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 1,
    GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1,
    None
  };
  dmnsn_display *display;
  XVisualInfo *vi;
  XSetWindowAttributes swa;

  display = malloc(sizeof(dmnsn_display));
  if (!display) {
    return NULL;
  }

  /* Get an X connection */
  display->dpy = XOpenDisplay(0);
  if (!display->dpy) {
    free(display);
    return NULL;
  }

  /* Get an appropriate visual */
  vi = glXChooseVisual(display->dpy, DefaultScreen(display->dpy),
                       attributeList);
  if (!vi) {
    XCloseDisplay(display->dpy);
    free(display);
    return NULL;
  }

  /* Create a GLX context */
  display->cx = glXCreateContext(display->dpy, vi, 0, GL_TRUE);
  if (!display->cx) {
    XCloseDisplay(display->dpy);
    free(display);
    return NULL;
  }

  /* Create a color map */
  display->cmap = XCreateColormap(display->dpy,
                                  RootWindow(display->dpy, vi->screen),
                                  vi->visual, AllocNone);
  if (!display->cmap) {
    glXDestroyContext(display->dpy, display->cx);
    XCloseDisplay(display->dpy);
    free(display);
    return NULL;
  }

  /* Create a window */
  swa.colormap = display->cmap;
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask;
  display->win = XCreateWindow(display->dpy,
                               RootWindow(display->dpy, vi->screen),
                               0, 0, canvas->x, canvas->y,
                               0, vi->depth, InputOutput, vi->visual,
                               CWBorderPixel|CWColormap|CWEventMask, &swa);
  if (!display->win) {
    XFreeColormap(display->dpy, display->cmap);
    glXDestroyContext(display->dpy, display->cx);
    XCloseDisplay(display->dpy);
    free(display);
    return NULL;
  }

  XStoreName(display->dpy, display->win, "glX");

  XMapWindow(display->dpy, display->win);
  XIfEvent(display->dpy, &display->event, WaitForNotify, (char*)display->win);

  /* Connect the context to the window */
  glXMakeCurrent(display->dpy, display->win, display->cx);

  return display;
}

void
dmnsn_delete_display(dmnsn_display *display)
{
  if (display) {
    XDestroyWindow(display->dpy, display->win);
    XFreeColormap(display->dpy, display->cmap);
    if (display->cx) {
      glXDestroyContext(display->dpy, display->cx);
    }
    XCloseDisplay(display->dpy);
    free(display);
  }
}

void
dmnsn_display_frame(dmnsn_display *display)
{
  if (display->cx) {
    glFlush();
    glXSwapBuffers(display->dpy, display->win);
  }
  XSync(display->dpy, True);
}

/* Print a progress bar of the progress of `progress' */
void
progressbar(const char *str, const dmnsn_progress *progress)
{
  const unsigned int increments = 32;
  unsigned int i;

  printf("%s|", str);
  fflush(stdout);
  for (i = 0; i < increments; ++i) {
    dmnsn_wait_progress(progress, ((double)(i + 1))/increments);

    printf("=");
    fflush(stdout);
  }
  printf("|\n");
  fflush(stdout);
}
