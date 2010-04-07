/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
  /* Allocate a new scene */
  dmnsn_scene *scene = dmnsn_new_scene();
  if (!scene) {
    return NULL;
  }

  /* Default finish */
  scene->default_texture->finish = dmnsn_new_ambient_finish(
    dmnsn_color_mul(0.1, dmnsn_white)
  );
  scene->default_texture->finish = dmnsn_new_finish_combination(
    dmnsn_new_diffuse_finish(0.6),
    scene->default_texture->finish
  );
  scene->default_texture->finish = dmnsn_new_finish_combination(
    dmnsn_new_phong_finish(0.2, 40.0),
    scene->default_texture->finish
  );
  if (!scene->default_texture->finish) {
    dmnsn_delete_scene(scene);
    return NULL;
  }

  /* Background color */
  scene->background = dmnsn_color_mul(0.1, dmnsn_blue);
  scene->background.filter = 0.1;

  /* Allocate a canvas */
  scene->canvas = dmnsn_new_canvas(768, 480);
  if (!scene->canvas) {
    dmnsn_delete_scene(scene);
    return NULL;
  }

  /* Set up the transformation matrix for the perspective camera */
  dmnsn_matrix trans = dmnsn_scale_matrix(
    dmnsn_new_vector(
      ((double)scene->canvas->x)/scene->canvas->y, 1.0, 1.0
    )
  );
  trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(dmnsn_new_vector(0.0, 0.0, -4.0)),
    trans
  );
  trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(0.0, 1.0, 0.0)),
    trans
  );

  /* Create a perspective camera */
  scene->camera = dmnsn_new_perspective_camera();
  if (!scene->camera) {
    dmnsn_delete_scene(scene);
    return NULL;
  }
  dmnsn_set_perspective_camera_trans(scene->camera, trans);

  /* Now make our objects */

  dmnsn_object *sphere = dmnsn_new_sphere();
  if (!sphere) {
    dmnsn_delete_scene(scene);
    return NULL;
  }

  sphere->texture = dmnsn_new_texture();
  if (!sphere->texture) {
    dmnsn_delete_scene(scene);
    return NULL;
  }

  sphere->texture->pigment = dmnsn_new_solid_pigment(dmnsn_yellow);
  if (!sphere->texture->pigment) {
    dmnsn_delete_scene(scene);
    return NULL;
  }

  sphere->trans = dmnsn_scale_matrix(dmnsn_new_vector(1.25, 1.25, 1.25));

  dmnsn_object *cube = dmnsn_new_cube();
  if (!cube) {
    dmnsn_delete_object(sphere);
    dmnsn_delete_scene(scene);
    return NULL;
  }

  cube->texture = dmnsn_new_texture();
  if (!cube->texture) {
    dmnsn_delete_object(sphere);
    dmnsn_delete_scene(scene);
    return NULL;
  }

  dmnsn_color cube_color = dmnsn_magenta;
  cube_color.filter = 0.25;
  cube_color.trans  = 0.5;
  cube->texture->pigment = dmnsn_new_solid_pigment(cube_color);
  if (!cube->texture->pigment) {
    dmnsn_delete_object(sphere);
    dmnsn_delete_scene(scene);
    return NULL;
  }

  dmnsn_color reflect = dmnsn_color_mul(0.5, dmnsn_white);
  cube->texture->finish = dmnsn_new_reflective_finish(reflect, reflect, 1.0);
  if (!cube->texture->finish) {
    dmnsn_delete_object(sphere);
    dmnsn_delete_scene(scene);
    return NULL;
  }

  cube->interior = dmnsn_new_interior();
  if (!cube->interior) {
    dmnsn_delete_object(sphere);
    dmnsn_delete_scene(scene);
    return NULL;
  }
  cube->interior->ior = 1.1;

  cube->trans = dmnsn_rotation_matrix(dmnsn_new_vector(0.75, 0.0, 0.0));

  dmnsn_object *csg = dmnsn_new_csg_difference(cube, sphere);
  if (!csg) {
    dmnsn_delete_scene(scene);
    return NULL;
  }
  dmnsn_array_push(scene->objects, &csg);

  /* Now make a light */

  dmnsn_light *light = dmnsn_new_point_light(
    dmnsn_new_vector(-15.0, 20.0, 10.0),
    dmnsn_cyan
  );
  if (!light) {
    dmnsn_delete_scene(scene);
    return NULL;
  }
  dmnsn_array_push(scene->lights, &light);

  return scene;
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
  XSetWindowAttributes swa;
  dmnsn_display *display;

  display = malloc(sizeof(dmnsn_display));
  if (!display) {
    return NULL;
  }

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
                               0, 0, canvas->x, canvas->y,
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

    free(display);
  }
}

void
dmnsn_display_flush(dmnsn_display *display)
{
  glXWaitX();
  glXSwapBuffers(display->dpy, display->win);
}
