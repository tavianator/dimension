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

#include "dimension-internal.h"
#include <stdlib.h>

/*
 * Boilerplate for multithreading
 */

/** Payload type for passing arguments to worker threads. */
typedef struct {
  dmnsn_future *future;
  dmnsn_scene *scene;
  dmnsn_prtree *prtree;
} dmnsn_raytrace_payload;

/* Raytrace a scene */
void
dmnsn_raytrace_scene(dmnsn_scene *scene)
{
  dmnsn_future *future = dmnsn_raytrace_scene_async(scene);
  if (dmnsn_future_join(future) != 0) {
    dmnsn_error("Error occured while raytracing.");
  }
}

/** Background thread callback. */
static int dmnsn_raytrace_scene_thread(void *ptr);

/* Raytrace a scene in the background */
dmnsn_future *
dmnsn_raytrace_scene_async(dmnsn_scene *scene)
{
  dmnsn_future *future = dmnsn_new_future();

  dmnsn_raytrace_payload *payload =
    dmnsn_malloc(sizeof(dmnsn_raytrace_payload));
  payload->future = future;
  payload->scene  = scene;

  dmnsn_new_thread(future, dmnsn_raytrace_scene_thread, payload);

  return future;
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
  dmnsn_start_timer(&payload->scene->bounding_timer);
    payload->prtree = dmnsn_new_prtree(payload->scene->objects);
  dmnsn_stop_timer(&payload->scene->bounding_timer);

  /* Set up the future object */
  dmnsn_future_set_total(payload->future, payload->scene->canvas->height);

  /* Time the render itself */
  dmnsn_start_timer(&payload->scene->render_timer);
    int ret = dmnsn_execute_concurrently(dmnsn_raytrace_scene_concurrent,
                                         payload, payload->scene->nthreads);
  dmnsn_stop_timer(&payload->scene->render_timer);

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
  const dmnsn_texture *texture;
  const dmnsn_interior *interior;
  const dmnsn_prtree *prtree;
  unsigned int reclevel;

  dmnsn_vector r;
  dmnsn_vector pigment_r;
  dmnsn_vector viewer;
  dmnsn_vector reflected;
  dmnsn_vector light;

  dmnsn_color pigment;
  dmnsn_color diffuse;
  dmnsn_color additional;

  double ior;

  dmnsn_color adc_value;
} dmnsn_raytrace_state;

/** Compute a raytrace state from an intersection. */
static inline void
dmnsn_initialize_raytrace_state(dmnsn_raytrace_state *state,
                                const dmnsn_intersection *intersection)
{
  state->intersection = intersection;
  state->texture      = intersection->object->texture;
  state->interior     = intersection->object->interior;

  state->r = dmnsn_line_point(intersection->ray, intersection->t);
  state->pigment_r = dmnsn_transform_point(
    intersection->object->pigment_trans,
    state->r
  );
  state->viewer = dmnsn_vector_normalized(
    dmnsn_vector_negate(intersection->ray.n)
  );
  state->reflected = dmnsn_vector_sub(
    dmnsn_vector_mul(
      2*dmnsn_vector_dot(state->viewer, intersection->normal),
      intersection->normal),
    state->viewer
  );

  state->pigment    = dmnsn_black;
  state->diffuse    = dmnsn_black;
  state->additional = dmnsn_black;
}

/** Main helper for dmnsn_raytrace_scene_impl - shoot a ray. */
static dmnsn_color dmnsn_raytrace_shoot(dmnsn_raytrace_state *state,
                                        dmnsn_line ray);

/* Actually raytrace a scene */
static int
dmnsn_raytrace_scene_concurrent(void *ptr, unsigned int thread,
                                unsigned int nthreads)
{
  const dmnsn_raytrace_payload *payload = ptr;
  dmnsn_future *future = payload->future;
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
        ((double)(x + scene->region_x))/(scene->outer_width - 1),
        ((double)(y + scene->region_y))/(scene->outer_height - 1)
      );

      /* Shoot a ray */
      state.reclevel = scene->reclimit;
      state.ior = 1.0;
      state.adc_value = dmnsn_white;
      dmnsn_color color = dmnsn_raytrace_shoot(&state, ray);

      dmnsn_set_pixel(scene->canvas, x, y, color);
    }

    dmnsn_future_increment(future);
  }

  return 0;
}

/** Calculate the background color. */
static dmnsn_color
dmnsn_raytrace_background(const dmnsn_raytrace_state *state, dmnsn_line ray)
{
  dmnsn_pigment *background = state->scene->background;
  if (state->scene->quality & DMNSN_RENDER_PIGMENT) {
    return dmnsn_evaluate_pigment(background, dmnsn_vector_normalized(ray.n));
  } else {
    return background->quick_color;
  }
}

/** Calculate the base pigment at the intersection. */
static void
dmnsn_raytrace_pigment(dmnsn_raytrace_state *state)
{
  dmnsn_pigment *pigment = state->texture->pigment;
  if (state->scene->quality & DMNSN_RENDER_PIGMENT) {
    state->pigment = dmnsn_evaluate_pigment(pigment, state->pigment_r);
  } else {
    state->pigment = pigment->quick_color;
  }

  state->diffuse = state->pigment;
}

/** Get the color of a light ray at an intersection point. */
static dmnsn_color
dmnsn_raytrace_light_ray(dmnsn_raytrace_state *state,
                         const dmnsn_light *light)
{
  /** @todo: Start at the light source */
  dmnsn_line shadow_ray = dmnsn_new_line(
    state->r,
    light->direction_fn(light, state->r)
  );
  state->light = dmnsn_vector_normalized(shadow_ray.n);
  /* Add epsilon to avoid hitting ourselves with the shadow ray */
  shadow_ray = dmnsn_line_add_epsilon(shadow_ray);

  /* Check if we're casting a shadow on ourself */
  if (dmnsn_vector_dot(shadow_ray.n, state->intersection->normal)
      * dmnsn_vector_dot(state->viewer, state->intersection->normal) < 0.0)
    return dmnsn_black;

  dmnsn_color color = light->illumination_fn(light, state->r);

  /* Test for shadow ray intersections */
  unsigned int reclevel = state->reclevel;
  while (reclevel-- > 0
         && dmnsn_color_intensity(color) >= state->scene->adc_bailout)
  {
    dmnsn_intersection shadow_caster;
    bool shadow_was_cast = dmnsn_prtree_intersection(state->prtree, shadow_ray,
                                                     &shadow_caster, false);

    if (!shadow_was_cast || !light->shadow_fn(light, shadow_caster.t)) {
      return color;
    }

    /* Handle transparency */
    if (state->scene->quality & DMNSN_RENDER_TRANSPARENCY) {
      dmnsn_raytrace_state shadow_state = *state;
      dmnsn_initialize_raytrace_state(&shadow_state, &shadow_caster);
      dmnsn_raytrace_pigment(&shadow_state);

      if (shadow_state.pigment.trans >= dmnsn_epsilon) {
        /* Reflect the light */
        const dmnsn_reflection *reflection =
          shadow_state.texture->finish.reflection;
        if ((state->scene->quality & DMNSN_RENDER_REFLECTION) && reflection) {
          dmnsn_color reflected = reflection->reflection_fn(
            reflection, color, shadow_state.pigment, shadow_state.reflected,
            shadow_state.intersection->normal
          );
          color = dmnsn_color_sub(color, reflected);
        }

        /* Filter the light */
        color = dmnsn_filter_light(color, shadow_state.pigment);
        shadow_ray.x0 = dmnsn_line_point(shadow_ray, shadow_caster.t);
        shadow_ray.n  = light->direction_fn(light, shadow_ray.x0);
        shadow_ray = dmnsn_line_add_epsilon(shadow_ray);
        continue;
      }
    }

    break;
  }

  return dmnsn_black;
}

/** Handle light, shadow, and shading. */
static void
dmnsn_raytrace_lighting(dmnsn_raytrace_state *state)
{
  /* The ambient color */
  state->diffuse = dmnsn_black;

  const dmnsn_finish *finish = &state->texture->finish;
  if (finish->ambient) {
    state->diffuse =
      finish->ambient->ambient_fn(finish->ambient, state->pigment);
  }

  /* Iterate over each light */
  DMNSN_ARRAY_FOREACH (dmnsn_light **, light, state->scene->lights) {
    dmnsn_color light_color = dmnsn_raytrace_light_ray(state, *light);
    if (!dmnsn_color_is_black(light_color)) {
      if (state->scene->quality & DMNSN_RENDER_FINISH) {
        /* Reflect the light */
        const dmnsn_reflection *reflection = state->texture->finish.reflection;
        if ((state->scene->quality & DMNSN_RENDER_REFLECTION) && reflection) {
          dmnsn_color reflected = reflection->reflection_fn(
            reflection, light_color, state->pigment, state->reflected,
            state->intersection->normal
          );
          light_color = dmnsn_color_sub(light_color, reflected);
        }

        /* Get this light's color contribution to the object */
        dmnsn_color diffuse = dmnsn_black;
        if (finish->diffuse) {
          diffuse = finish->diffuse->diffuse_fn(
            finish->diffuse, light_color, state->pigment, state->light,
            state->intersection->normal
          );
        }

        dmnsn_color specular = dmnsn_black;
        if (finish->specular) {
          specular = finish->specular->specular_fn(
            finish->specular, light_color, state->pigment, state->light,
            state->intersection->normal, state->viewer
          );
        }

        state->diffuse = dmnsn_color_add(diffuse, state->diffuse);
        state->additional = dmnsn_color_add(specular, state->additional);
      } else {
        state->diffuse = state->pigment;
        state->diffuse.trans  = 0.0;
        state->diffuse.filter = 0.0;
      }
    }
  }
}

/** Trace a reflected ray. */
static dmnsn_color
dmnsn_raytrace_reflection(const dmnsn_raytrace_state *state)
{
  dmnsn_color reflected = dmnsn_black;

  const dmnsn_reflection *reflection = state->texture->finish.reflection;
  if (reflection) {
    dmnsn_line refl_ray = dmnsn_new_line(state->r, state->reflected);
    refl_ray = dmnsn_line_add_epsilon(refl_ray);

    dmnsn_raytrace_state recursive_state = *state;

    /* Calculate ADC value */
    recursive_state.adc_value = reflection->reflection_fn(
      reflection, state->adc_value, state->pigment, state->reflected,
      state->intersection->normal
    );

    /* Shoot the reflected ray */
    dmnsn_color rec = dmnsn_raytrace_shoot(&recursive_state, refl_ray);
    reflected = reflection->reflection_fn(
      reflection, rec, state->pigment, state->reflected,
      state->intersection->normal
    );
    reflected.trans  = 0.0;
    reflected.filter = 0.0;
  }

  return reflected;
}

/** Handle transparency - must be called last to work correctly. */
static void
dmnsn_raytrace_transparency(dmnsn_raytrace_state *state)
{
  if (state->pigment.trans >= dmnsn_epsilon) {
    dmnsn_line trans_ray = dmnsn_new_line(state->r, state->intersection->ray.n);
    trans_ray = dmnsn_line_add_epsilon(trans_ray);

    dmnsn_vector r = dmnsn_vector_normalized(trans_ray.n);
    dmnsn_vector n = state->intersection->normal;

    dmnsn_raytrace_state recursive_state = *state;

    /* Calculate new refractive index */
    if (dmnsn_vector_dot(r, n) < 0.0) {
      /* We are entering an object */
      recursive_state.ior = state->interior->ior;
      recursive_state.parent = state;
    } else {
      /* We are leaving an object */
      recursive_state.ior = state->parent ? state->parent->ior : 1.0;
      recursive_state.parent = state->parent ? state->parent->parent : NULL;
    }

    /* Calculate transmitted ray direction */
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

    /* Calculate ADC value */
    recursive_state.adc_value =
      dmnsn_filter_light(state->adc_value, state->pigment);

    /* Shoot the transmitted ray */
    dmnsn_color rec = dmnsn_raytrace_shoot(&recursive_state, trans_ray);
    dmnsn_color filtered = dmnsn_filter_light(rec, state->pigment);

    /* Conserve energy */
    const dmnsn_reflection *reflection = state->texture->finish.reflection;
    if ((state->scene->quality & DMNSN_RENDER_REFLECTION) && reflection) {
      dmnsn_color reflected = reflection->reflection_fn(
        reflection, filtered, state->pigment, state->reflected,
        state->intersection->normal
      );
      filtered = dmnsn_color_sub(filtered, reflected);
    }

    state->diffuse.filter = state->pigment.filter;
    state->diffuse.trans  = state->pigment.trans;
    state->diffuse = dmnsn_apply_transparency(filtered, state->diffuse);
  }
}

/* Shoot a ray, and calculate the color */
static dmnsn_color
dmnsn_raytrace_shoot(dmnsn_raytrace_state *state, dmnsn_line ray)
{
  if (state->reclevel == 0
      || dmnsn_color_intensity(state->adc_value) < state->scene->adc_bailout)
  {
    return dmnsn_black;
  }

  --state->reclevel;

  dmnsn_intersection intersection;
  bool reset = state->reclevel == state->scene->reclimit - 1;
  if (dmnsn_prtree_intersection(state->prtree, ray, &intersection, reset)) {
    /* Found an intersection */
    dmnsn_initialize_raytrace_state(state, &intersection);

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

    /* Transparency */
    if (state->scene->quality & DMNSN_RENDER_TRANSPARENCY) {
      dmnsn_raytrace_transparency(state);
    }

    return dmnsn_color_add(state->diffuse, state->additional);
  } else {
    /* No intersection, return the background color */
    return dmnsn_raytrace_background(state, ray);
  }
}
