/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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

dmnsn_scene *
dmnsn_new_default_scene()
{
  dmnsn_scene *scene;
  dmnsn_object *sphere, *cube;
  dmnsn_matrix trans;
  dmnsn_sRGB sRGB;
  dmnsn_color color;

  /* Allocate our new scene */
  scene = dmnsn_new_scene();
  if (!scene) {
    return NULL;
  }

  /* Background color */
  sRGB.R = 0.0;
  sRGB.G = 0.0;
  sRGB.B = 0.1;
  color = dmnsn_color_from_sRGB(sRGB);
  color.filter = 0.1;
  scene->background = color;

  /* Allocate a canvas */
  scene->canvas = dmnsn_new_canvas(768, 480);
  if (!scene->canvas) {
    dmnsn_delete_scene(scene);
    return NULL;
  }

  /* Set up the transformation matrix for the perspective camera */
  trans = dmnsn_scale_matrix(
    dmnsn_vector_construct(
      ((double)scene->canvas->x)/scene->canvas->y, 1.0, 1.0
    )
  );
  trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(dmnsn_vector_construct(0.0, 0.0, -4.0)),
    trans
  );
  trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_vector_construct(0.0, 1.0, 0.0)),
    trans
  );

  /* Create a perspective camera */
  scene->camera = dmnsn_new_perspective_camera(trans);
  if (!scene->camera) {
    dmnsn_delete_canvas(scene->canvas);
    dmnsn_delete_scene(scene);
    return NULL;
  }

  /* Now make our objects */

  sphere = dmnsn_new_sphere();
  if (!sphere) {
    dmnsn_delete_perspective_camera(scene->camera);
    dmnsn_delete_canvas(scene->canvas);
    dmnsn_delete_scene(scene);
    return NULL;
  }

  sphere->trans = dmnsn_matrix_inverse(
    dmnsn_scale_matrix(dmnsn_vector_construct(1.25, 1.25, 1.25))
  );
  dmnsn_array_push(scene->objects, &sphere);

  cube = dmnsn_new_cube();
  if (!cube) {
    dmnsn_delete_sphere(sphere);
    dmnsn_delete_perspective_camera(scene->camera);
    dmnsn_delete_canvas(scene->canvas);
    dmnsn_delete_scene(scene);
    return NULL;
  }

  cube->trans = dmnsn_matrix_inverse(
    dmnsn_rotation_matrix(dmnsn_vector_construct(0.75, 0.0, 0.0))
  );
  dmnsn_array_push(scene->objects, &cube);

  return scene;
}

void
dmnsn_delete_default_scene(dmnsn_scene *scene)
{
  dmnsn_object *sphere, *cube;
  dmnsn_array_get(scene->objects, 0, &sphere);
  dmnsn_array_get(scene->objects, 1, &cube);

  dmnsn_delete_cube(cube);
  dmnsn_delete_sphere(sphere);
  dmnsn_delete_perspective_camera(scene->camera);
  dmnsn_delete_canvas(scene->canvas);
  dmnsn_delete_scene(scene);
}

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
    glXDestroyContext(display->dpy, display->cx);
    XCloseDisplay(display->dpy);
    free(display);
  }
}

void
dmnsn_display_frame(dmnsn_display *display)
{
  glFlush();
  glXSwapBuffers(display->dpy, display->win);
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
