/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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

  /* For multithreading */
  unsigned int index, threads;
} dmnsn_raytrace_payload;

/* Thread callback */
static void *dmnsn_raytrace_scene_thread(void *ptr);

/* Raytrace a scene */
int
dmnsn_raytrace_scene(dmnsn_scene *scene)
{
  dmnsn_progress *progress = dmnsn_raytrace_scene_async(scene);
  return dmnsn_finish_progress(progress);
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
                       payload) != 0)
    {
      free(payload);
      dmnsn_delete_progress(progress);
      return NULL;
    }
  }

  return progress;
}

/* Start the multi-threaded implementation */
static int dmnsn_raytrace_scene_multithread(dmnsn_raytrace_payload *payload);

/* Thread callback */
static void *
dmnsn_raytrace_scene_thread(void *ptr)
{
  dmnsn_raytrace_payload *payload = ptr;
  int *retval = malloc(sizeof(int));
  if (retval) {
    *retval = dmnsn_raytrace_scene_multithread(payload);
  }
  dmnsn_done_progress(payload->progress);
  free(payload);
  return retval;
}

/* Thread callback */
static void *dmnsn_raytrace_scene_multithread_thread(void *ptr);

/* Set up the multi-threaded engine */
static int
dmnsn_raytrace_scene_multithread(dmnsn_raytrace_payload *payload)
{
  int i, j, nthreads;
  void *ptr;
  int retval = 0;
  dmnsn_raytrace_payload *payloads;
  pthread_t *threads;

  /* Find the number of processors/cores running (TODO: do this portably) */
  nthreads = sysconf(_SC_NPROCESSORS_ONLN);
  if (nthreads < 1) {
    nthreads = 1;
  }
  /* End non-portable section */

  payloads = malloc(nthreads*sizeof(dmnsn_raytrace_payload));
  if (!payloads) {
    return 1;
  }

  threads = malloc(nthreads*sizeof(pthread_t));
  if (!threads) {
    free(payloads);
    return 1;
  }

  /* Set up the progress object */
  dmnsn_new_progress_element(payload->progress,
                             nthreads*payload->scene->canvas->y);

  /* Create the threads */
  for (i = 0; i < nthreads; ++i) {
    payloads[i] = *payload;
    payloads[i].index = i;
    payloads[i].threads = nthreads;

    if (pthread_create(&threads[i], NULL,
                       &dmnsn_raytrace_scene_multithread_thread,
                       &payloads[i]) != 0)
    {
      for (j = 0; j < i; ++j) {
        if (pthread_join(threads[i], &ptr)) {
          dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                      "Couldn't join worker thread in failed raytrace engine"
                      " initialization.");
        } else {
          free(ptr);
        }
      }

      free(payloads);
      return 1;
    }
  }

  for (i = 0; i < nthreads; ++i) {
    if (pthread_join(threads[i], &ptr)) {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                  "Couldn't join worker thread in raytrace engine.");
    } else {
      if (retval == 0) {
        retval = *(int *)ptr;
      }
      free(ptr);
    }
  }

  free(threads);
  free(payloads);
  return retval;
}

/* Actual raytracing implementation */
static int dmnsn_raytrace_scene_impl(dmnsn_progress *progress,
                                     dmnsn_scene *scene,
                                     unsigned int index, unsigned int threads);

/* Multi-threading thread callback */
static void *
dmnsn_raytrace_scene_multithread_thread(void *ptr)
{
  dmnsn_raytrace_payload *payload = ptr;
  int *retval = malloc(sizeof(int));
  if (retval) {
    *retval = dmnsn_raytrace_scene_impl(payload->progress, payload->scene,
                                        payload->index, payload->threads);
  }
  return retval;
}

/* Helper for dmnsn_raytrace_scene_impl - shoot a ray */
static dmnsn_color dmnsn_raytrace_shoot(dmnsn_scene *scene, dmnsn_color color,
                                        dmnsn_line ray);

/*
 * Actually raytrace a scene
 */
static int
dmnsn_raytrace_scene_impl(dmnsn_progress *progress, dmnsn_scene *scene,
                          unsigned int index, unsigned int threads)
{
  unsigned int x, y;
  unsigned int width, height;
  dmnsn_line ray;
  dmnsn_color color;

  width  = scene->canvas->x;
  height = scene->canvas->y;

  /* Initialize `x' */
  x = width + index;

  /* Iterate through each pixel */
  for (y = 0; y < height; ++y) {
    for (x -= width; x < width; x += threads) {
      /* Set the pixel to the background color */
      color = scene->background;

      if (scene->quality >= DMNSN_RENDER_OBJECTS) {
        /* Get the ray corresponding to the (x,y)'th pixel */
        ray = (*scene->camera->ray_fn)(scene->camera, scene->canvas, x, y);
        /* Shoot a ray */
        color = dmnsn_raytrace_shoot(scene, color, ray);
      }

      dmnsn_set_pixel(scene->canvas, x, y, color);
    }

    dmnsn_increment_progress(progress);
  }

  return 0;
}

/* Shoot a ray, and calculate the color, using `color' as the background */
static dmnsn_color
dmnsn_raytrace_shoot(dmnsn_scene *scene, dmnsn_color color, dmnsn_line ray)
{
  dmnsn_line ray_trans;
  dmnsn_object *object;
  dmnsn_intersection *intersection = NULL, *intersection_temp;
  const dmnsn_texture *texture;
  const dmnsn_pigment *pigment;
  unsigned int i;

  for (i = 0; i < dmnsn_array_size(scene->objects); ++i) {
    dmnsn_array_get(scene->objects, i, &object);

    /* Transform the ray according to the object */
    ray_trans = dmnsn_matrix_line_mul(object->trans, ray);

    /* Test for intersections with objects */
    intersection_temp = (*object->intersection_fn)(object, ray_trans);

    /* Find the closest intersection to the camera */
    if (intersection_temp &&
        (!intersection || intersection_temp->t < intersection->t)) {
      dmnsn_delete_intersection(intersection);
      intersection = intersection_temp;
    }
  }

  if (intersection) {
    /* Default to black if we have no texture/pigment */
    color = dmnsn_black;

    if (scene->quality >= DMNSN_RENDER_PIGMENT) {
      /* Use the default texture if given a NULL texture */
      texture = intersection->texture ? intersection->texture
                                      : scene->default_texture;

      if (texture) {
        /* Use the default pigment if given a NULL pigment */
        pigment = texture->pigment ? texture->pigment
                                   : scene->default_texture->pigment;

        if (pigment) {
          color = (*pigment->pigment_fn)(
            pigment,
            dmnsn_line_point(intersection->ray, intersection->t)
          );
        }
      }
    }

    /* Delete the intersection */
    dmnsn_delete_intersection(intersection);
  }

  return color;
}
