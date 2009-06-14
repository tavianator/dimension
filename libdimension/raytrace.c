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
#include <unistd.h> /* For sysconf */

typedef struct {
  dmnsn_scene *scene;
  unsigned int i, n;
} dmnsn_raytrace_thread_payload;

static void *dmnsn_raytrace_scene_thread(void *arg);

void
dmnsn_raytrace_scene(dmnsn_scene *scene)
{
  long n = sysconf(_SC_NPROCESSORS_ONLN);
  unsigned int i;
  pthread_t thread;
  dmnsn_raytrace_thread_payload payload;
  dmnsn_array *threads, *payloads;

  threads  = dmnsn_new_array(sizeof(pthread_t));
  payloads = dmnsn_new_array(sizeof(dmnsn_raytrace_thread_payload));

  if (n <= 0) n = 1;

  payload.scene = scene;
  payload.n = n;
  for (i = 0; i < n; ++i) {
    payload.i = i;
    dmnsn_array_push(payloads, &payload);

    pthread_create(&thread, NULL, &dmnsn_raytrace_scene_thread,
                   dmnsn_array_at(payloads, i));
    dmnsn_array_push(threads, &thread);
  }

  for (i = 0; i < n; ++i) {
    dmnsn_array_get(threads, i, &thread);
    pthread_join(thread, NULL);
  }

  dmnsn_delete_array(payloads);
  dmnsn_delete_array(threads);
}

/* Raytrace a scene */
static void *
dmnsn_raytrace_scene_thread(void *arg)
{
  unsigned int i, j, k, l;
  double t, t_temp;
  dmnsn_object *object;
  dmnsn_line ray, ray_trans;
  dmnsn_raytrace_thread_payload *payload = (dmnsn_raytrace_thread_payload *)arg;
  dmnsn_scene *scene = payload->scene;
  dmnsn_array *intersections;
  dmnsn_color color;
  dmnsn_sRGB sRGB;

  /* Iterate through each pixel */
  for (i = 0; i < scene->canvas->x; ++i) {
    for (j = 0; j < scene->canvas->y; ++j) {
      /* Only do the pixels assigned to this thread */
      if ((j*scene->canvas->x + i)%payload->n == payload->i) {
        /* Set the pixel to the background color */
        color = scene->background;
        t = 0.0;

        /* Get the ray corresponding to the (i,j)th pixel */
        ray = (*scene->camera->ray_fn)(scene->camera, scene->canvas, i, j);

        for (k = 0; k < scene->objects->length; ++k) {
          dmnsn_array_get(scene->objects, k, &object);

          /* Transform the ray according to the object */
          ray_trans = dmnsn_matrix_line_mul(object->trans, ray);

          /* Test for intersections with objects */
          intersections = (*object->intersections_fn)(object, ray_trans);
          for (l = 0; l < intersections->length; ++l) {
            dmnsn_array_get(intersections, l, &t_temp);
            if (t_temp < t || t == 0.0) t = t_temp;
          }
          dmnsn_delete_array(intersections);
        }

        if (t != 0.0) {
          sRGB.R = 1.0 - (t - 2.25)/2.25;
          sRGB.G = sRGB.R;
          sRGB.B = sRGB.R;
          color = dmnsn_color_from_sRGB(sRGB);
        }

        dmnsn_set_pixel(scene->canvas, i, j, color);
      }
    }
  }

  return NULL;
}
