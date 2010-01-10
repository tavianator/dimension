/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
#include <stdbool.h>

/*
 * Boilerplate for multithreading
 */

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
                             payload->scene->canvas->y);

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

/*
 * Raytracing algorithm
 */

typedef struct dmnsn_raytrace_state {
  const dmnsn_scene *scene;
  const dmnsn_intersection *intersection;
  dmnsn_kD_splay_tree *kD_splay_tree;
  unsigned int level;

  dmnsn_color pigment;
} dmnsn_raytrace_state;

/* Main helper for dmnsn_raytrace_scene_impl - shoot a ray */
static dmnsn_color dmnsn_raytrace_shoot(dmnsn_raytrace_state *state,
                                        dmnsn_line ray);

/* Actually raytrace a scene */
static int
dmnsn_raytrace_scene_impl(dmnsn_progress *progress, dmnsn_scene *scene,
                          dmnsn_kD_splay_tree *kD_splay_tree,
                          unsigned int index, unsigned int threads)
{
  dmnsn_raytrace_state state = {
    .scene = scene,
    .kD_splay_tree = kD_splay_tree
  };

  unsigned int width  = scene->canvas->x;
  unsigned int height = scene->canvas->y;

  /* Iterate through each pixel */
  unsigned int y;
  for (y = index; y < height; y += threads) {
    unsigned int x;
    for (x = 0; x < width; ++x) {
      /* Set the pixel to the background color */
      dmnsn_color color = scene->background;

      if (scene->quality) {
        /* Get the ray corresponding to the (x,y)'th pixel */
        dmnsn_line ray = (*scene->camera->ray_fn)(
          scene->camera,
          ((double)x)/(scene->canvas->x - 1),
          ((double)y)/(scene->canvas->y - 1)
        );

        /* Shoot a ray */
        state.level = scene->limit;
        color = dmnsn_raytrace_shoot(&state, ray);
      }

      dmnsn_set_pixel(scene->canvas, x, y, color);
    }

    dmnsn_increment_progress(progress);
  }

  return 0;
}

/* Add epsilon*l.n to l.x0, to avoid self-intersections */
static dmnsn_line
dmnsn_line_add_epsilon(dmnsn_line l)
{
  return dmnsn_new_line(
    dmnsn_vector_add(
      l.x0,
      dmnsn_vector_mul(1.0e-9, l.n)
    ),
    l.n
  );
}

static const dmnsn_pigment *
dmnsn_raytrace_get_pigment(const dmnsn_raytrace_state *state)
{
  /* Use the default texture if given a NULL texture */
  const dmnsn_texture *texture
    = state->intersection->texture ? state->intersection->texture
                                   : state->scene->default_texture;

  if (texture) {
    /* Use the default pigment if given a NULL pigment */
    return texture->pigment ? texture->pigment
                            : state->scene->default_texture->pigment;
  } else {
    return NULL;
  }
}

static const dmnsn_finish *
dmnsn_raytrace_get_finish(const dmnsn_raytrace_state *state)
{
  /* Use the default texture if given a NULL texture */
  const dmnsn_texture *texture
    = state->intersection->texture ? state->intersection->texture
                                   : state->scene->default_texture;

  if (texture) {
    /* Use the default finish if given a NULL finish */
    return texture->finish ? texture->finish
                           : state->scene->default_texture->finish;
  } else {
    return NULL;
  }
}

static dmnsn_color
dmnsn_raytrace_pigment(const dmnsn_raytrace_state *state)
{
  /* Default to black if there's no texture/pigment */
  dmnsn_color color = dmnsn_black;

  const dmnsn_pigment *pigment = dmnsn_raytrace_get_pigment(state);
  if (pigment) {
    color = (*pigment->pigment_fn)(
      pigment,
      dmnsn_line_point(state->intersection->ray, state->intersection->t)
    );
  }

  return color;
}

/* Get the color of a light ray at an intersection point */
static bool
dmnsn_raytrace_light_ray(const dmnsn_raytrace_state *state,
                         const dmnsn_light *light, dmnsn_color *color)
{
  *color = dmnsn_black;

  dmnsn_vector x0 = dmnsn_line_point(state->intersection->ray,
                                     state->intersection->t);
  dmnsn_line shadow_ray = dmnsn_new_line(x0, dmnsn_vector_sub(light->x0, x0));
  /* Add epsilon to avoid hitting ourselves with the shadow ray */
  shadow_ray = dmnsn_line_add_epsilon(shadow_ray);

  /* Search for an object in the way of the light source */
  if (dmnsn_vector_dot(shadow_ray.n, state->intersection->normal) < 0.0)
    return false;

  *color = (*light->light_fn)(light, x0);

  unsigned int level = state->level;
  while (level) {
    dmnsn_intersection *shadow_caster = dmnsn_kD_splay_search(
      state->kD_splay_tree,
      shadow_ray
    );

    if (!shadow_caster || shadow_caster->t > 1.0) {
      dmnsn_delete_intersection(shadow_caster);
      break;
    }

    dmnsn_raytrace_state shadow_state = *state;
    shadow_state.intersection = shadow_caster;
    shadow_state.level        = level;

    dmnsn_color pigment = dmnsn_raytrace_pigment(&shadow_state);
    if (pigment.filter || pigment.trans) {
      *color = dmnsn_color_filter(*color, pigment);
      shadow_ray.x0 = dmnsn_line_point(shadow_ray, shadow_caster->t);
      shadow_ray.n  = dmnsn_vector_sub(light->x0, shadow_ray.x0);
      shadow_ray = dmnsn_line_add_epsilon(shadow_ray);
    } else {
      *color = dmnsn_black;
      dmnsn_delete_intersection(shadow_caster);
      return false;
    }

    dmnsn_delete_intersection(shadow_caster);
    --level;
  }

  return true;
}

static dmnsn_color
dmnsn_raytrace_lighting(const dmnsn_raytrace_state *state)
{
  const dmnsn_finish *finish = dmnsn_raytrace_get_finish(state);

  /* The illuminated color */
  dmnsn_color illum = dmnsn_black;
  if (finish && finish->ambient_fn)
    illum = (*finish->ambient_fn)(finish, state->pigment);

  dmnsn_vector x0 = dmnsn_line_point(state->intersection->ray,
                                     state->intersection->t);

  const dmnsn_light *light;
  unsigned int i;

  /* Iterate over each light */
  for (i = 0; i < dmnsn_array_size(state->scene->lights); ++i) {
    dmnsn_array_get(state->scene->lights, i, &light);

    dmnsn_color light_color;
    if (dmnsn_raytrace_light_ray(state, light, &light_color)) {
      if (state->scene->quality & DMNSN_RENDER_FINISH
          && finish && finish->finish_fn)
      {
        dmnsn_vector ray = dmnsn_vector_normalize(
          dmnsn_vector_sub(light->x0, x0)
        );
        dmnsn_vector normal = state->intersection->normal;
        dmnsn_vector viewer = dmnsn_vector_normalize(
          dmnsn_vector_negate(state->intersection->ray.n)
        );

        /* Get this light's color contribution to the object */
        dmnsn_color contrib = (*finish->finish_fn)(finish,
                                                   light_color, state->pigment,
                                                   ray, normal, viewer);
        illum = dmnsn_color_add(contrib, illum);
      } else {
        illum = state->pigment;
      }
    }
  }

  return illum;
}

static dmnsn_color
dmnsn_raytrace_reflection(const dmnsn_raytrace_state *state, dmnsn_color color)
{
  const dmnsn_finish *finish = dmnsn_raytrace_get_finish(state);
  dmnsn_color refl = color;

  if (finish && finish->reflection_fn) {
    dmnsn_vector normal = state->intersection->normal;
    dmnsn_vector viewer = dmnsn_vector_normalize(
      dmnsn_vector_negate(state->intersection->ray.n)
    );
    dmnsn_vector ray = dmnsn_vector_sub(
      dmnsn_vector_mul(2*dmnsn_vector_dot(viewer, normal), normal),
      viewer
    );

    dmnsn_line refl_ray = dmnsn_new_line(
      dmnsn_line_point(state->intersection->ray, state->intersection->t),
      ray
    );
    refl_ray = dmnsn_line_add_epsilon(refl_ray);

    dmnsn_raytrace_state recursive_state = *state;
    dmnsn_color rec = dmnsn_raytrace_shoot(&recursive_state, refl_ray);
    refl = dmnsn_color_add(
      (*finish->reflection_fn)(finish, rec, state->pigment, ray, normal),
      refl
    );
  }

  return refl;
}

static dmnsn_color
dmnsn_raytrace_translucency(const dmnsn_raytrace_state *state,
                            dmnsn_color color)
{
  dmnsn_color trans = color;
  if (state->pigment.filter || state->pigment.trans) {
    trans = dmnsn_color_mul(1.0 - state->pigment.filter - state->pigment.trans,
                            color);

    dmnsn_line trans_ray = dmnsn_new_line(
      dmnsn_line_point(state->intersection->ray, state->intersection->t),
      state->intersection->ray.n
    );
    trans_ray = dmnsn_line_add_epsilon(trans_ray);

    dmnsn_raytrace_state recursive_state = *state;
    dmnsn_color rec = dmnsn_raytrace_shoot(&recursive_state, trans_ray);
    dmnsn_color filtered = dmnsn_color_filter(rec, state->pigment);
    trans = dmnsn_color_add(trans, filtered);
  }
  return trans;
}

/* Shoot a ray, and calculate the color, using `color' as the background */
static dmnsn_color
dmnsn_raytrace_shoot(dmnsn_raytrace_state *state, dmnsn_line ray)
{
  if (state->level <= 0)
    return dmnsn_black;
  --state->level;

  dmnsn_intersection *intersection
    = dmnsn_kD_splay_search(state->kD_splay_tree, ray);

  dmnsn_color color = state->scene->background;
  if (intersection) {
    state->intersection = intersection;

    /* Pigment */
    state->pigment = dmnsn_black;
    if (state->scene->quality & DMNSN_RENDER_PIGMENT) {
      state->pigment = dmnsn_raytrace_pigment(state);
    }
    color = state->pigment;

    /* Finishes and shadows */
    dmnsn_color illum = color;
    if (state->scene->quality & DMNSN_RENDER_LIGHTS) {
      illum = dmnsn_raytrace_lighting(state);
    }
    color = illum;

    /* Reflection */
    dmnsn_color refl = illum;
    if (state->scene->quality & DMNSN_RENDER_REFLECTION) {
      refl = dmnsn_raytrace_reflection(state, refl);
    }
    color = refl;

    /* Translucency */
    dmnsn_color trans = color;
    trans.filter = 0.0;
    trans.trans  = 0.0;
    if (state->scene->quality & DMNSN_RENDER_TRANSLUCENCY) {
      trans = dmnsn_raytrace_translucency(state, trans);
    }
    color = trans;

    dmnsn_delete_intersection(intersection);
  }

  return color;
}
