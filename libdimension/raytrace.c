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

#include "dimension_impl.h"
#include <unistd.h> /* For sysconf */

/* Payload type for passing arguments to worker thread */

typedef struct {
  dmnsn_progress *progress;
  dmnsn_scene *scene;
  dmnsn_kD_splay_tree *kD_splay_tree;

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
  unsigned int i;
  dmnsn_object *object;
  dmnsn_raytrace_payload *payload;
  dmnsn_progress *progress = dmnsn_new_progress();

  if (progress) {
    payload = malloc(sizeof(dmnsn_raytrace_payload));
    if (!payload) {
      dmnsn_delete_progress(progress);
      return NULL;
    }

    payload->progress      = progress;
    payload->scene         = scene;
    payload->kD_splay_tree = dmnsn_new_kD_splay_tree();

    for (i = 0; i < dmnsn_array_size(payload->scene->objects); ++i) {
      dmnsn_array_get(payload->scene->objects, i, &object);
      dmnsn_kD_splay_insert(payload->kD_splay_tree, object);
    }

    if (pthread_create(&progress->thread, NULL, &dmnsn_raytrace_scene_thread,
                       payload) != 0)
    {
      dmnsn_delete_kD_splay_tree(payload->kD_splay_tree);
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

  /* Create the payloads */
  for (i = 0; i < nthreads; ++i) {
    payloads[i] = *payload;
    payloads[i].index = i;
    payloads[i].threads = nthreads;
    if (i > 0) {
      payloads[i].kD_splay_tree =
        dmnsn_kD_splay_copy(payloads[0].kD_splay_tree);
    }
  }

  /* Create the threads */
  for (i = 0; i < nthreads; ++i) {
    if (pthread_create(&threads[i], NULL,
                       &dmnsn_raytrace_scene_multithread_thread,
                       &payloads[i]) != 0)
    {
      for (j = 0; j < i; ++j) {
        if (pthread_join(threads[j], &ptr)) {
          dmnsn_error(DMNSN_SEVERITY_MEDIUM,
                      "Couldn't join worker thread in failed raytrace engine"
                      " initialization.");
        } else {
          /* Only free on a successful join - otherwise we might free a pointer
             out from under a running thread */
          dmnsn_delete_kD_splay_tree(payloads[j].kD_splay_tree);
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
      dmnsn_delete_kD_splay_tree(payloads[i].kD_splay_tree);
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
                                     dmnsn_kD_splay_tree *kD_splay_tree,
                                     unsigned int index, unsigned int threads);

/* Multi-threading thread callback */
static void *
dmnsn_raytrace_scene_multithread_thread(void *ptr)
{
  dmnsn_raytrace_payload *payload = ptr;
  int *retval = malloc(sizeof(int));
  if (retval) {
    *retval = dmnsn_raytrace_scene_impl(payload->progress, payload->scene,
                                        payload->kD_splay_tree,
                                        payload->index, payload->threads);
  }
  return retval;
}

/* Helper for dmnsn_raytrace_scene_impl - shoot a ray */
static dmnsn_color dmnsn_raytrace_shoot(dmnsn_line ray, dmnsn_scene *scene,
                                        dmnsn_kD_splay_tree *kD_splay_tree,
                                        dmnsn_color color);

/*
 * Actually raytrace a scene
 */
static int
dmnsn_raytrace_scene_impl(dmnsn_progress *progress, dmnsn_scene *scene,
                          dmnsn_kD_splay_tree *kD_splay_tree,
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
        ray = (*scene->camera->ray_fn)(scene->camera,
                                       ((double)x)/(scene->canvas->x - 1),
                                       ((double)y)/(scene->canvas->y - 1));
        /* Shoot a ray */
        color = dmnsn_raytrace_shoot(ray, scene, kD_splay_tree, color);
      }

      dmnsn_set_pixel(scene->canvas, x, y, color);
    }

    dmnsn_increment_progress(progress);
  }

  return 0;
}

/* Shoot a ray, and calculate the color, using `color' as the background */
static dmnsn_color
dmnsn_raytrace_shoot(dmnsn_line ray, dmnsn_scene *scene,
                     dmnsn_kD_splay_tree *kD_splay_tree, dmnsn_color color)
{
  dmnsn_intersection *intersection = dmnsn_kD_splay_search(kD_splay_tree, ray);

  if (intersection) {
    /* Default to black if we have no texture/pigment */
    color = dmnsn_black;

    if (scene->quality >= DMNSN_RENDER_PIGMENT) {
      /* Use the default texture if given a NULL texture */
      const dmnsn_texture *texture
        = intersection->texture ? intersection->texture
                                : scene->default_texture;

      if (texture) {
        /* Use the default pigment if given a NULL pigment */
        const dmnsn_pigment *pigment
          = texture->pigment ? texture->pigment
                             : scene->default_texture->pigment;

        if (pigment) {
          color = (*pigment->pigment_fn)(
            pigment,
            dmnsn_line_point(intersection->ray, intersection->t)
          );
        }
      }
    }

    if (scene->quality >= DMNSN_RENDER_LIGHTS) {
      dmnsn_color illum = dmnsn_color_mul(0.3, color);

      const dmnsn_light *light;
      unsigned int i;

      for (i = 0; i < dmnsn_array_size(scene->lights); ++i) {
        dmnsn_array_get(scene->lights, i, &light);
        dmnsn_vector x0 = dmnsn_line_point(ray, intersection->t);
        dmnsn_line shadow_ray = dmnsn_line_construct(
          dmnsn_vector_add(
            x0,
            dmnsn_vector_mul(1.0e-6, dmnsn_vector_sub(light->x0, x0))
          ),
          dmnsn_vector_sub(light->x0, x0)
        );

        dmnsn_intersection *shadow_caster
          = dmnsn_kD_splay_search(kD_splay_tree, shadow_ray);

        if (!shadow_caster || shadow_caster->t > 1.0) {
          dmnsn_vector object_normal = intersection->normal;
          dmnsn_vector normal = dmnsn_vector_normalize(
            dmnsn_vector_add(
              dmnsn_vector_normalize(dmnsn_vector_sub(ray.x0, x0)),
              dmnsn_vector_normalize(dmnsn_vector_sub(light->x0, x0))
            )
          );

          illum = dmnsn_color_add(
            dmnsn_color_mul(
              dmnsn_vector_dot(normal, object_normal),
              dmnsn_color_illuminate((*light->light_fn)(light, x0), color)
            ),
            illum
          );
        }

        dmnsn_delete_intersection(shadow_caster);
      }

      color = illum;
    }

    /* Delete the intersection */
    dmnsn_delete_intersection(intersection);
  }

  return color;
}
