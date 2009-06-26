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

/* Payload type for passing arguments to worker thread */

typedef struct {
  dmnsn_progress *progress;
  dmnsn_scene *scene;
} dmnsn_raytrace_payload;

/* Thread callback */
static void *dmnsn_raytrace_scene_thread(void *ptr);

/* Raytrace a scene */
void
dmnsn_raytrace_scene(dmnsn_scene *scene)
{
  dmnsn_progress *progress = dmnsn_raytrace_scene_async(scene);
  dmnsn_finish_progress(progress);
}

/* Raytrace a scene in the background */
dmnsn_progress *
dmnsn_raytrace_scene_async(dmnsn_scene *scene)
{
  dmnsn_progress *progress = dmnsn_new_progress();
  dmnsn_raytrace_payload *payload;

  if (progress) {
    payload = malloc(sizeof(dmnsn_raytrace_payload));
    if (!payload) {
      dmnsn_delete_progress(progress);
      return NULL;
    }

    payload->progress = progress;
    payload->scene    = scene;

    if (pthread_create(&progress->thread, NULL, &dmnsn_raytrace_scene_thread,
                       payload)
        != 0) {
      free(payload);
      dmnsn_delete_progress(progress);
      return NULL;
    }
  }

  return progress;
}

/* Actual raytracing implementation */
static void dmnsn_raytrace_scene_impl(dmnsn_progress *progress,
                                      dmnsn_scene *scene);

/* Thread callback */
static void *
dmnsn_raytrace_scene_thread(void *ptr)
{
  dmnsn_raytrace_payload *payload = ptr;
  int *retval = malloc(sizeof(int));
  if (retval) {
    dmnsn_raytrace_scene_impl(payload->progress, payload->scene);
    *retval = 0;
  }
  dmnsn_done_progress(payload->progress);
  free(payload);
  return retval;
}

/* Actually raytrace a scene */
static void
dmnsn_raytrace_scene_impl(dmnsn_progress *progress, dmnsn_scene *scene)
{
  unsigned int i, j, k, l;
  unsigned int width, height;
  double t, t_temp;
  dmnsn_object *object;
  dmnsn_line ray, ray_trans;
  dmnsn_array *intersections;
  dmnsn_color color;
  dmnsn_sRGB sRGB;

  width  = scene->canvas->x;
  height = scene->canvas->y;

  dmnsn_new_progress_element(progress, height);

  /* Iterate through each pixel */
  for (j = 0; j < height; ++j) {
    for (i = 0; i < width; ++i) {
      /* Set the pixel to the background color */
      color = scene->background;
      t = 0.0;

      /* Get the ray corresponding to the (i,j)th pixel */
      ray = (*scene->camera->ray_fn)(scene->camera, scene->canvas, i, j);

      for (k = 0; k < dmnsn_array_size(scene->objects); ++k) {
        dmnsn_array_get(scene->objects, k, &object);

        /* Transform the ray according to the object */
        ray_trans = dmnsn_matrix_line_mul(object->trans, ray);

        /* Test for intersections with objects */
        intersections = (*object->intersections_fn)(object, ray_trans);
        /* Find the closest intersection */
        for (l = 0; l < dmnsn_array_size(intersections); ++l) {
          dmnsn_array_get(intersections, l, &t_temp);
          if (t_temp < t || t == 0.0) t = t_temp;
        }
        dmnsn_delete_array(intersections);
      }

      /* Shade according to distance from camera */
      if (t != 0.0) {
        sRGB.R = 1.0 - (t - 2.25)/2.25;
        sRGB.G = sRGB.R;
        sRGB.B = sRGB.R;
        color = dmnsn_color_from_sRGB(sRGB);
      }

      dmnsn_set_pixel(scene->canvas, i, j, color);
    }

    dmnsn_increment_progress(progress);
  }
}
