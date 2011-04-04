/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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

/**
 * @file
 * The ray-tracing algorithm.
 */

#include "dimension-impl.h"
#include <stdlib.h>

/*
 * Boilerplate for multithreading
 */

/** Payload type for passing arguments to worker threads. */
typedef struct {
  dmnsn_progress *progress;
  dmnsn_scene *scene;
  dmnsn_prtree *prtree;
} dmnsn_raytrace_payload;

/* Raytrace a scene */
void
dmnsn_raytrace_scene(dmnsn_scene *scene)
{
  dmnsn_progress *progress = dmnsn_raytrace_scene_async(scene);
  if (dmnsn_finish_progress(progress) != 0) {
    dmnsn_error("Error occured while raytracing.");
  }
}

/** Background thread callback. */
static int dmnsn_raytrace_scene_thread(void *ptr);

/* Raytrace a scene in the background */
dmnsn_progress *
dmnsn_raytrace_scene_async(dmnsn_scene *scene)
{
  dmnsn_progress *progress = dmnsn_new_progress();

  dmnsn_raytrace_payload *payload
    = dmnsn_malloc(sizeof(dmnsn_raytrace_payload));
  payload->progress = progress;
  payload->scene    = scene;

  dmnsn_new_thread(progress, dmnsn_raytrace_scene_thread, payload);

  return progress;
}

/** Worker thread callback. */
static int dmnsn_raytrace_scene_concurrent(void *ptr, unsigned int thread,
                                           unsigned int nthreads);

/* Thread callback -- set up the multithreaded engine */
static int
dmnsn_raytrace_scene_thread(void *ptr)
{
  dmnsn_raytrace_payload *payload = ptr;

  /* Pre-calculate bounding box transformations, etc. */
  dmnsn_initialize_scene(payload->scene);

  /* Time the bounding tree construction */
  payload->scene->bounding_timer = dmnsn_new_timer();
    payload->prtree = dmnsn_new_prtree(payload->scene->objects);
  dmnsn_complete_timer(payload->scene->bounding_timer);

  /* Set up the progress object */
  dmnsn_set_progress_total(payload->progress, payload->scene->canvas->height);

  /* Time the render itself */
  payload->scene->render_timer = dmnsn_new_timer();
    int ret = dmnsn_execute_concurrently(dmnsn_raytrace_scene_concurrent,
                                         payload, payload->scene->nthreads);
  dmnsn_complete_timer(payload->scene->render_timer);

  dmnsn_delete_prtree(payload->prtree);
  dmnsn_free(payload);

  return ret;
}

/*
 * Raytracing algorithm
 */

/** The current state of the ray-tracing engine. */
typedef struct dmnsn_raytrace_state {
  const struct dmnsn_raytrace_state *parent;

  const dmnsn_scene *scene;
  const dmnsn_intersection *intersection;
  dmnsn_prtree *prtree;
  unsigned int reclevel;

  dmnsn_vector r;
  dmnsn_vector viewer;
  dmnsn_vector reflected;

  dmnsn_color pigment;
  dmnsn_color diffuse;
  dmnsn_color additional;

  double ior;
} dmnsn_raytrace_state;

/** Main helper for dmnsn_raytrace_scene_impl - shoot a ray. */
static dmnsn_color dmnsn_raytrace_shoot(dmnsn_raytrace_state *state,
                                        dmnsn_line ray);

/* Actually raytrace a scene */
static int
dmnsn_raytrace_scene_concurrent(void *ptr, unsigned int thread,
                                unsigned int nthreads)
{
  const dmnsn_raytrace_payload *payload = ptr;
  dmnsn_progress *progress = payload->progress;
  dmnsn_scene *scene = payload->scene;
  dmnsn_prtree *prtree = payload->prtree;

  dmnsn_raytrace_state state = {
    .parent = NULL,
    .scene  = scene,
    .prtree = prtree,
  };

  /* Iterate through each pixel */
  for (size_t y = thread; y < scene->canvas->height; y += nthreads) {
    for (size_t x = 0; x < scene->canvas->width; ++x) {
      /* Get the ray corresponding to the (x,y)'th pixel */
      dmnsn_line ray = dmnsn_camera_ray(
        scene->camera,
        ((double)x)/(scene->canvas->width - 1),
        ((double)y)/(scene->canvas->height - 1)
      );

      /* Shoot a ray */
      state.reclevel = scene->reclimit;
      state.ior = 1.0;
      dmnsn_color color = dmnsn_raytrace_shoot(&state, ray);

      dmnsn_set_pixel(scene->canvas, x, y, color);
    }

    dmnsn_increment_progress(progress);
  }

  return 0;
}

/** Get the intersection texture. */
#define ITEXTURE(state) (state->intersection->texture)
/** Get the default texture. */
#define DTEXTURE(state) (state->scene->default_texture)

/** Can a texture element be accessed?. */
#define CAN_ACCESS(texture, telem)              \
  ((texture) && (texture)->telem)
/** Can a texture element callback be called?. */
#define CAN_CALL(texture, telem, fn)                    \
  (CAN_ACCESS(texture, telem) && (texture)->telem->fn)

/** Determine whether a callback may be called. */
#define TEXTURE_HAS_CALLBACK(state, telem, fn)  \
  (CAN_CALL(ITEXTURE(state), telem, fn)         \
   || CAN_CALL(DTEXTURE(state), telem, fn))

/** Call the appropriate overloaded texture callback. */
#define TEXTURE_CALLBACK(state, telem, fn, def, ...)                       \
  (CAN_CALL(ITEXTURE(state), telem, fn)                                    \
   ? ITEXTURE(state)->telem->fn(ITEXTURE(state)->telem, __VA_ARGS__)    \
   : (CAN_CALL(DTEXTURE(state), telem, fn)                                 \
      ? DTEXTURE(state)->telem->fn(DTEXTURE(state)->telem, __VA_ARGS__) \
      : def));

/** Get a property from a texture element. */
#define TEXTURE_PROPERTY(state, telem, prop, def)       \
  (CAN_ACCESS(ITEXTURE(state), telem)                   \
   ? ITEXTURE(state)->telem->prop                       \
   : (CAN_ACCESS(DTEXTURE(state), telem)                \
      ? DTEXTURE(state)->telem->prop                    \
      : def))

/** Get the current index of refraction. */
#define IOR(state)                              \
  ((state)->intersection->interior              \
   ? (state)->intersection->interior->ior       \
   : 1.0)

/** Calculate the background color. */
static dmnsn_color
dmnsn_raytrace_background(dmnsn_raytrace_state *state, dmnsn_line ray)
{
  dmnsn_color color = state->scene->background;
  if (state->scene->sky_sphere
      && (state->scene->quality & DMNSN_RENDER_PIGMENT))
  {
    dmnsn_color sky = dmnsn_sky_sphere_color(state->scene->sky_sphere,
                                             dmnsn_vector_normalize(ray.n));
    color = dmnsn_color_add(dmnsn_color_filter(color, sky), sky);
  }

  return color;
}

/** Calculate the base pigment at the intersection. */
static void
dmnsn_raytrace_pigment(dmnsn_raytrace_state *state)
{
  if (state->scene->quality & DMNSN_RENDER_PIGMENT) {
    state->pigment = TEXTURE_CALLBACK(state, pigment, pigment_fn, dmnsn_black,
                                      state->r);
  } else {
    state->pigment = TEXTURE_PROPERTY(state, pigment, quick_color, dmnsn_black);
  }
  state->diffuse = state->pigment;
}

/** Get the color of a light ray at an intersection point. */
static dmnsn_color
dmnsn_raytrace_light_ray(const dmnsn_raytrace_state *state,
                         const dmnsn_light *light)
{
  dmnsn_line shadow_ray = dmnsn_new_line(state->r,
                                         dmnsn_vector_sub(light->x0, state->r));
  /* Add epsilon to avoid hitting ourselves with the shadow ray */
  shadow_ray = dmnsn_line_add_epsilon(shadow_ray);

  /* Check if we're casting a shadow on ourself */
  if (dmnsn_vector_dot(shadow_ray.n, state->intersection->normal)
      * dmnsn_vector_dot(state->viewer, state->intersection->normal) < 0.0)
    return dmnsn_black;

  dmnsn_color color = light->light_fn(light, state->r);

  unsigned int reclevel = state->reclevel;
  while (reclevel) {
    dmnsn_intersection shadow_caster;
    bool shadow_casted
      = dmnsn_prtree_intersection(state->prtree, shadow_ray, &shadow_caster);

    if (!shadow_casted || shadow_caster.t > 1.0) {
      break;
    }

    dmnsn_raytrace_state shadow_state = *state;
    shadow_state.intersection = &shadow_caster;
    shadow_state.reclevel     = reclevel;

    dmnsn_raytrace_pigment(&shadow_state);
    if ((state->scene->quality & DMNSN_RENDER_TRANSLUCENCY)
        && (shadow_state.pigment.filter || shadow_state.pigment.trans)) {
      color = dmnsn_color_filter(color, shadow_state.pigment);
      shadow_ray.x0 = dmnsn_line_point(shadow_ray, shadow_caster.t);
      shadow_ray.n  = dmnsn_vector_sub(light->x0, shadow_ray.x0);
      shadow_ray = dmnsn_line_add_epsilon(shadow_ray);
    } else {
      return dmnsn_black;
    }

    --reclevel;
  }

  return color;
}

/** Handle light, shadow, and shading. */
static void
dmnsn_raytrace_lighting(dmnsn_raytrace_state *state)
{
  /* The ambient color */
  state->diffuse = TEXTURE_CALLBACK(state, finish, ambient_fn, dmnsn_black,
                                    state->pigment);
  state->diffuse = dmnsn_color_illuminate(state->scene->ambient,
                                          state->diffuse);

  if (!TEXTURE_HAS_CALLBACK(state, finish, diffuse_fn)
      && !TEXTURE_HAS_CALLBACK(state, finish, specular_fn))
  {
    return;
  }

  /* Iterate over each light */
  DMNSN_ARRAY_FOREACH (dmnsn_light **, light, state->scene->lights) {
    dmnsn_color light_color = dmnsn_raytrace_light_ray(state, *light);
    if (!dmnsn_color_is_black(light_color)) {
      if (state->scene->quality & DMNSN_RENDER_FINISH) {
        dmnsn_vector ray = dmnsn_vector_normalize(
          dmnsn_vector_sub((*light)->x0, state->r)
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
        state->diffuse.filter = state->diffuse.trans = 0.0;
      }
    }
  }
}

/** Trace a reflected ray. */
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

/** Handle translucency - must be called last to work correctly. */
static void
dmnsn_raytrace_translucency(dmnsn_raytrace_state *state)
{
  if (state->pigment.filter || state->pigment.trans) {
    dmnsn_line trans_ray = dmnsn_new_line(state->r, state->intersection->ray.n);
    trans_ray = dmnsn_line_add_epsilon(trans_ray);

    dmnsn_vector r = dmnsn_vector_normalize(trans_ray.n);
    dmnsn_vector n = state->intersection->normal;

    dmnsn_raytrace_state recursive_state = *state;

    if (dmnsn_vector_dot(r, n) < 0.0) {
      /* We are entering an object */
      recursive_state.ior = IOR(state);
      recursive_state.parent = state;
    } else {
      /* We are leaving an object */
      recursive_state.ior = state->parent ? state->parent->ior : 1.0;
      recursive_state.parent = state->parent ? state->parent->parent : NULL;
    }

    double iorr = state->ior/recursive_state.ior; /* ior ratio */

    double c1 = -dmnsn_vector_dot(r, n);
    double c2 = 1.0 - iorr*iorr*(1.0 - c1*c1);
    if (c2 <= 0.0) {
      /* Total internal reflection */
      return;
    }
    c2 = sqrt(c2);

    if (c1 >= 0.0) {
      trans_ray.n = dmnsn_vector_add(
        dmnsn_vector_mul(iorr, r),
        dmnsn_vector_mul(iorr*c1 - c2, n)
      );
    } else {
      trans_ray.n = dmnsn_vector_add(
        dmnsn_vector_mul(iorr, r),
        dmnsn_vector_mul(iorr*c1 + c2, n)
      );
    }

    state->diffuse = dmnsn_color_mul(
      1.0 - state->pigment.filter - state->pigment.trans,
      state->diffuse
    );

    dmnsn_color rec = dmnsn_raytrace_shoot(&recursive_state, trans_ray);
    dmnsn_color filtered = dmnsn_color_filter(rec, state->pigment);
    state->additional = dmnsn_color_add(filtered, state->additional);
  }
}

/* Shoot a ray, and calculate the color */
static dmnsn_color
dmnsn_raytrace_shoot(dmnsn_raytrace_state *state, dmnsn_line ray)
{
  if (state->reclevel <= 0)
    return dmnsn_black;
  --state->reclevel;

  /* Calculate the background color */
  dmnsn_color color = dmnsn_raytrace_background(state, ray);

  dmnsn_intersection intersection;
  if (dmnsn_prtree_intersection(state->prtree, ray, &intersection)) {
    state->intersection = &intersection;
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
    dmnsn_raytrace_pigment(state);

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
  }

  return color;
}
