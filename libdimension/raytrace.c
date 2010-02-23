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

/*
 * Boilerplate for multithreading
 */

/* Payload type for passing arguments to worker thread */

typedef struct {
  dmnsn_progress *progress;
  dmnsn_scene *scene;
  dmnsn_bvst *bvst;

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

    payload->progress = progress;
    payload->scene    = scene;
    payload->bvst     = dmnsn_new_bvst();

    for (i = 0; i < dmnsn_array_size(payload->scene->objects); ++i) {
      dmnsn_array_get(payload->scene->objects, i, &object);
      dmnsn_bvst_insert(payload->bvst, object);
    }

    if (pthread_create(&progress->thread, NULL, &dmnsn_raytrace_scene_thread,
                       payload) != 0)
    {
      dmnsn_delete_bvst(payload->bvst);
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
  int i, j;
  void *ptr;
  int retval = 0;
  dmnsn_raytrace_payload *payloads;
  pthread_t *threads;

  unsigned int nthreads = payload->scene->nthreads;
  /* Sanity check */
  if (nthreads < 1)
    nthreads = 1;

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
      payloads[i].bvst = dmnsn_bvst_copy(payloads[0].bvst);
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
          dmnsn_delete_bvst(payloads[j].bvst);
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
      dmnsn_delete_bvst(payloads[i].bvst);
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
                                     dmnsn_bvst *bvst,
                                     unsigned int index, unsigned int threads);

/* Multi-threading thread callback */
static void *
dmnsn_raytrace_scene_multithread_thread(void *ptr)
{
  dmnsn_raytrace_payload *payload = ptr;
  int *retval = malloc(sizeof(int));
  if (retval) {
    *retval = dmnsn_raytrace_scene_impl(payload->progress, payload->scene,
                                        payload->bvst,
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
  dmnsn_bvst *bvst;
  unsigned int reclevel;

  dmnsn_vector r;
  dmnsn_vector viewer;
  dmnsn_vector reflected;

  dmnsn_color pigment;
  dmnsn_color diffuse;
  dmnsn_color additional;
} dmnsn_raytrace_state;

/* Main helper for dmnsn_raytrace_scene_impl - shoot a ray */
static dmnsn_color dmnsn_raytrace_shoot(dmnsn_raytrace_state *state,
                                        dmnsn_line ray);

/* Actually raytrace a scene */
static int
dmnsn_raytrace_scene_impl(dmnsn_progress *progress, dmnsn_scene *scene,
                          dmnsn_bvst *bvst,
                          unsigned int index, unsigned int threads)
{
  dmnsn_raytrace_state state = {
    .scene = scene,
    .bvst = bvst
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
        state.reclevel = scene->reclimit;
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

#define ITEXTURE(state) (state->intersection->texture)
#define DTEXTURE(state) (state->scene->default_texture)

#define CAN_CALL(texture, telem, fn)                            \
  ((texture) && (texture)->telem && (texture)->telem->fn)

/* Determine whether a callback may be called */
#define TEXTURE_HAS_CALLBACK(state, telem, fn)  \
  (CAN_CALL(ITEXTURE(state), telem, fn)         \
   || CAN_CALL(DTEXTURE(state), telem, fn))

/* Call the appropriate overloaded texture callback */
#define TEXTURE_CALLBACK(state, telem, fn, def, ...)                       \
  (CAN_CALL(ITEXTURE(state), telem, fn)                                    \
   ? (*ITEXTURE(state)->telem->fn)(ITEXTURE(state)->telem, __VA_ARGS__)    \
   : (CAN_CALL(DTEXTURE(state), telem, fn)                                 \
      ? (*DTEXTURE(state)->telem->fn)(DTEXTURE(state)->telem, __VA_ARGS__) \
      : def));                                                             \

static void
dmnsn_raytrace_pigment(dmnsn_raytrace_state *state)
{
  state->pigment = TEXTURE_CALLBACK(state, pigment, pigment_fn, dmnsn_black,
                                    state->r);
}

/* Get the color of a light ray at an intersection point */
static dmnsn_color
dmnsn_raytrace_light_ray(const dmnsn_raytrace_state *state,
                         const dmnsn_light *light)
{
  dmnsn_line shadow_ray = dmnsn_new_line(state->r,
                                         dmnsn_vector_sub(light->x0, state->r));
  /* Add epsilon to avoid hitting ourselves with the shadow ray */
  shadow_ray = dmnsn_line_add_epsilon(shadow_ray);

  /* Check if we're casting a shadow on ourself */
  if (dmnsn_vector_dot(shadow_ray.n, state->intersection->normal) < 0.0)
    return dmnsn_black;

  dmnsn_color color = (*light->light_fn)(light, state->r);

  unsigned int reclevel = state->reclevel;
  while (reclevel) {
    dmnsn_intersection *shadow_caster
      = dmnsn_bvst_search(state->bvst, shadow_ray);

    if (!shadow_caster || shadow_caster->t > 1.0) {
      dmnsn_delete_intersection(shadow_caster);
      break;
    }

    dmnsn_raytrace_state shadow_state = *state;
    shadow_state.intersection = shadow_caster;
    shadow_state.reclevel     = reclevel;

    dmnsn_raytrace_pigment(&shadow_state);
    if (shadow_state.pigment.filter || shadow_state.pigment.trans) {
      color = dmnsn_color_filter(color, shadow_state.pigment);
      shadow_ray.x0 = dmnsn_line_point(shadow_ray, shadow_caster->t);
      shadow_ray.n  = dmnsn_vector_sub(light->x0, shadow_ray.x0);
      shadow_ray = dmnsn_line_add_epsilon(shadow_ray);
    } else {
      dmnsn_delete_intersection(shadow_caster);
      return dmnsn_black;
    }

    dmnsn_delete_intersection(shadow_caster);
    --reclevel;
  }

  return color;
}

static void
dmnsn_raytrace_lighting(dmnsn_raytrace_state *state)
{
  /* The illuminated color */
  state->diffuse = TEXTURE_CALLBACK(state, finish, ambient_fn, dmnsn_black,
                                    state->pigment);

  if (!TEXTURE_HAS_CALLBACK(state, finish, diffuse_fn)
      && !TEXTURE_HAS_CALLBACK(state, finish, specular_fn))
  {
    return;
  }

  const dmnsn_light *light;
  unsigned int i;

  /* Iterate over each light */
  for (i = 0; i < dmnsn_array_size(state->scene->lights); ++i) {
    dmnsn_array_get(state->scene->lights, i, &light);

    dmnsn_color light_color = dmnsn_raytrace_light_ray(state, light);
    if (!dmnsn_color_is_black(light_color)) {
      if (state->scene->quality & DMNSN_RENDER_FINISH) {
        dmnsn_vector ray = dmnsn_vector_normalize(
          dmnsn_vector_sub(light->x0, state->r)
        );

        /* Get this light's color contribution to the object */
        dmnsn_color diffuse = TEXTURE_CALLBACK(
          state, finish, diffuse_fn, dmnsn_black,
          light_color, state->pigment, ray, state->intersection->normal
        );
        dmnsn_color specular = TEXTURE_CALLBACK(
          state, finish, specular_fn, dmnsn_black,
          light_color, state->pigment, ray, state->intersection->normal,
          state->viewer
        );
        state->diffuse = dmnsn_color_add(diffuse, state->diffuse);
        state->additional = dmnsn_color_add(specular, state->additional);
      } else {
        state->diffuse = state->pigment;
      }
    }
  }
}

static dmnsn_color
dmnsn_raytrace_reflection(const dmnsn_raytrace_state *state)
{
  dmnsn_color reflected = dmnsn_black;

  if (TEXTURE_HAS_CALLBACK(state, finish, reflection_fn)) {
    dmnsn_line refl_ray = dmnsn_new_line(state->r, state->reflected);
    refl_ray = dmnsn_line_add_epsilon(refl_ray);

    dmnsn_raytrace_state recursive_state = *state;
    dmnsn_color rec = dmnsn_raytrace_shoot(&recursive_state, refl_ray);
    reflected = TEXTURE_CALLBACK(
      state, finish, reflection_fn, dmnsn_black,
      rec, state->pigment, state->reflected, state->intersection->normal
    );
    reflected.filter = 0.0;
    reflected.trans  = 0.0;
  }

  return reflected;
}

/* Handle translucency - must be called last to work correctly */
static void
dmnsn_raytrace_translucency(dmnsn_raytrace_state *state)
{
  if (state->pigment.filter || state->pigment.trans) {
    state->diffuse = dmnsn_color_mul(
      1.0 - state->pigment.filter - state->pigment.trans,
      state->diffuse
    );

    dmnsn_line trans_ray = dmnsn_new_line(state->r, state->intersection->ray.n);
    trans_ray = dmnsn_line_add_epsilon(trans_ray);

    dmnsn_raytrace_state recursive_state = *state;
    dmnsn_color rec = dmnsn_raytrace_shoot(&recursive_state, trans_ray);
    dmnsn_color filtered = dmnsn_color_filter(rec, state->pigment);
    state->additional = dmnsn_color_add(filtered, state->additional);
  }
}

/* Shoot a ray, and calculate the color, using `color' as the background */
static dmnsn_color
dmnsn_raytrace_shoot(dmnsn_raytrace_state *state, dmnsn_line ray)
{
  if (state->reclevel <= 0)
    return dmnsn_black;
  --state->reclevel;

  dmnsn_intersection *intersection
    = dmnsn_bvst_search(state->bvst, ray);

  dmnsn_color color = state->scene->background;
  if (intersection) {
    state->intersection = intersection;
    state->r = dmnsn_line_point(state->intersection->ray,
                                state->intersection->t);
    state->viewer = dmnsn_vector_normalize(
      dmnsn_vector_negate(state->intersection->ray.n)
    );
    state->reflected = dmnsn_vector_sub(
      dmnsn_vector_mul(
        2*dmnsn_vector_dot(state->viewer, state->intersection->normal),
        state->intersection->normal),
      state->viewer
    );

    state->pigment    = dmnsn_black;
    state->diffuse    = dmnsn_black;
    state->additional = dmnsn_black;

    /* Pigment */
    if (state->scene->quality & DMNSN_RENDER_PIGMENT) {
      dmnsn_raytrace_pigment(state);
    }

    /* Finishes and shadows */
    if (state->scene->quality & DMNSN_RENDER_LIGHTS) {
      dmnsn_raytrace_lighting(state);
    }

    /* Reflection */
    if (state->scene->quality & DMNSN_RENDER_REFLECTION) {
      state->additional = dmnsn_color_add(
        dmnsn_raytrace_reflection(state),
        state->additional
      );
    }

    /* Translucency */
    if (state->scene->quality & DMNSN_RENDER_TRANSLUCENCY) {
      dmnsn_raytrace_translucency(state);
    }

    color = dmnsn_color_add(state->diffuse, state->additional);

    dmnsn_delete_intersection(intersection);
  }

  return color;
}
