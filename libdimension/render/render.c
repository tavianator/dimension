/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "internal/bvh.h"
#include "internal/concurrency.h"
#include "dimension/render.h"
#include <stdlib.h>

////////////////////////////////////
// Boilerplate for multithreading //
////////////////////////////////////

/// Payload type for passing arguments to worker threads.
typedef struct {
  dmnsn_future *future;
  dmnsn_scene *scene;
  dmnsn_bvh *bvh;
} dmnsn_render_payload;

// Ray-trace a scene
void
dmnsn_render(dmnsn_scene *scene)
{
  dmnsn_future *future = dmnsn_render_async(scene);
  if (dmnsn_future_join(future) != 0) {
    dmnsn_error("Error occured while ray-tracing.");
  }
}

/// Background thread callback.
static int dmnsn_render_scene_thread(void *ptr);

// Ray-trace a scene in the background
dmnsn_future *
dmnsn_render_async(dmnsn_scene *scene)
{
  dmnsn_future *future = dmnsn_new_future();

  dmnsn_render_payload *payload = DMNSN_MALLOC(dmnsn_render_payload);
  payload->future = future;
  payload->scene  = scene;

  dmnsn_new_thread(future, dmnsn_render_scene_thread, payload);

  return future;
}

/// Worker thread callback.
static int dmnsn_render_scene_concurrent(void *ptr, unsigned int thread,
                                            unsigned int nthreads);

// Thread callback -- set up the multithreaded engine
static int
dmnsn_render_scene_thread(void *ptr)
{
  dmnsn_render_payload *payload = ptr;

  // Pre-calculate bounding box transformations, etc.
  dmnsn_scene_initialize(payload->scene);

  // Time the bounding tree construction
  dmnsn_timer_start(&payload->scene->bounding_timer);
    payload->bvh = dmnsn_new_bvh(payload->scene->objects, DMNSN_BVH_PRTREE);
  dmnsn_timer_stop(&payload->scene->bounding_timer);

  // Set up the future object
  dmnsn_future_set_total(payload->future, payload->scene->canvas->height);

  // Time the render itself
  dmnsn_timer_start(&payload->scene->render_timer);
    int ret = dmnsn_execute_concurrently(payload->future,
                                         dmnsn_render_scene_concurrent,
                                         payload, payload->scene->nthreads);
  dmnsn_timer_stop(&payload->scene->render_timer);

  dmnsn_delete_bvh(payload->bvh);
  dmnsn_free(payload);

  return ret;
}

///////////////////////////
// Ray-tracing algorithm //
///////////////////////////

/// The current state of the ray-tracing engine.
typedef struct dmnsn_rtstate {
  const struct dmnsn_rtstate *parent;

  const dmnsn_scene *scene;
  const dmnsn_intersection *intersection;
  const dmnsn_texture *texture;
  const dmnsn_interior *interior;
  const dmnsn_bvh *bvh;
  unsigned int reclevel;

  dmnsn_vector r;
  dmnsn_vector pigment_r;
  dmnsn_vector viewer;
  dmnsn_vector reflected;

  bool is_shadow_ray;
  dmnsn_vector light_ray;
  dmnsn_color light_color;

  dmnsn_tcolor pigment;
  dmnsn_tcolor color;

  double ior;

  dmnsn_color adc_value;
} dmnsn_rtstate;

/// Compute a ray-tracing state from an intersection.
static inline void
dmnsn_rtstate_initialize(dmnsn_rtstate *state,
                         const dmnsn_intersection *intersection);
/// Main helper for dmnsn_render_scene_concurrent - shoot a ray.
static dmnsn_tcolor dmnsn_ray_shoot(dmnsn_rtstate *state, dmnsn_ray ray);

// Actually ray-trace a scene
static int
dmnsn_render_scene_concurrent(void *ptr, unsigned int thread, unsigned int nthreads)
{
  const dmnsn_render_payload *payload = ptr;
  dmnsn_future *future = payload->future;
  dmnsn_scene *scene = payload->scene;
  dmnsn_bvh *bvh = payload->bvh;

  dmnsn_rtstate state = {
    .parent = NULL,
    .scene  = scene,
    .bvh = bvh,
  };

  // Iterate through each pixel
  for (size_t y = thread; y < scene->canvas->height; y += nthreads) {
    for (size_t x = 0; x < scene->canvas->width; ++x) {
      // Get the ray corresponding to the (x,y)'th pixel
      dmnsn_ray ray = dmnsn_camera_ray(
        scene->camera,
        ((double)(x + scene->region_x))/(scene->outer_width - 1),
        ((double)(y + scene->region_y))/(scene->outer_height - 1)
      );

      // Shoot a ray
      state.reclevel = scene->reclimit;
      state.ior = 1.0;
      state.adc_value = dmnsn_white;
      dmnsn_tcolor tcolor = dmnsn_ray_shoot(&state, ray);
      dmnsn_canvas_set_pixel(scene->canvas, x, y, tcolor);
    }

    dmnsn_future_increment(future);
  }

  return 0;
}

// Compute rtstate fields
static inline void
dmnsn_rtstate_initialize(dmnsn_rtstate *state,
                         const dmnsn_intersection *intersection)
{
  state->intersection = intersection;
  state->texture      = intersection->object->texture;
  state->interior     = intersection->object->interior;

  state->r = dmnsn_ray_point(intersection->ray, intersection->t);
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

  state->is_shadow_ray = false;
}

/// Calculate the background color.
static void dmnsn_trace_background(dmnsn_rtstate *state, dmnsn_ray ray);
/// Calculate the base pigment at the intersection.
static void dmnsn_trace_pigment(dmnsn_rtstate *state);
/// Handle light, shadow, and shading.
static void dmnsn_trace_lighting(dmnsn_rtstate *state);
/// Trace a reflected ray.
static void dmnsn_trace_reflection(dmnsn_rtstate *state);
/// Trace a transmitted ray.
static void dmnsn_trace_transparency(dmnsn_rtstate *state);

// Shoot a ray, and calculate the color
static dmnsn_tcolor
dmnsn_ray_shoot(dmnsn_rtstate *state, dmnsn_ray ray)
{
  if (state->reclevel == 0
      || dmnsn_color_intensity(state->adc_value) < state->scene->adc_bailout)
  {
    return DMNSN_TCOLOR(dmnsn_black);
  }

  --state->reclevel;

  dmnsn_intersection intersection;
  bool reset = state->reclevel == state->scene->reclimit - 1;
  dmnsn_bvh_intersection(state->bvh, ray, &intersection, reset);
  if (dmnsn_bvh_intersection(state->bvh, ray, &intersection, reset)) {
    // Found an intersection
    dmnsn_rtstate_initialize(state, &intersection);

    dmnsn_trace_pigment(state);
    if (state->scene->quality & DMNSN_RENDER_LIGHTS) {
      dmnsn_trace_lighting(state);
    }
    if (state->scene->quality & DMNSN_RENDER_REFLECTION) {
      dmnsn_trace_reflection(state);
    }
    if (state->scene->quality & DMNSN_RENDER_TRANSPARENCY) {
      dmnsn_trace_transparency(state);
    }
  } else {
    // No intersection, return the background color
    dmnsn_trace_background(state, ray);
  }

  return state->color;
}

static void
dmnsn_trace_background(dmnsn_rtstate *state, dmnsn_ray ray)
{
  dmnsn_pigment *background = state->scene->background;
  if (state->scene->quality & DMNSN_RENDER_PIGMENT) {
    dmnsn_vector r = dmnsn_vector_normalized(ray.n);
    state->color = dmnsn_pigment_evaluate(background, r);
  } else {
    state->color = background->quick_color;
  }
}

static void
dmnsn_trace_pigment(dmnsn_rtstate *state)
{
  dmnsn_pigment *pigment = state->texture->pigment;
  if (state->scene->quality & DMNSN_RENDER_PIGMENT) {
    state->pigment = dmnsn_pigment_evaluate(pigment, state->pigment_r);
  } else {
    state->pigment = pigment->quick_color;
  }
  state->color = state->pigment;
}

/// Determine the amount of specular highlight.
static inline dmnsn_color
dmnsn_evaluate_specular(const dmnsn_rtstate *state)
{
  const dmnsn_finish *finish = &state->texture->finish;
  if (finish->specular) {
    return finish->specular->specular_fn(
      finish->specular, state->light_color, state->pigment.c,
      state->light_ray, state->intersection->normal, state->viewer
    );
  } else {
    return dmnsn_black;
  }
}

/// Determine the amount of reflected light.
static inline dmnsn_color
dmnsn_evaluate_reflection(const dmnsn_rtstate *state,
                          dmnsn_color light, dmnsn_vector direction)
{
  const dmnsn_reflection *reflection = state->texture->finish.reflection;
  if (reflection && (state->scene->quality & DMNSN_RENDER_REFLECTION)) {
    return reflection->reflection_fn(
      reflection, light, state->pigment.c, direction,
      state->intersection->normal
    );
  } else {
    return dmnsn_black;
  }
}

/// Determine the amount of transmitted light.
static inline dmnsn_color
dmnsn_evaluate_transparency(const dmnsn_rtstate *state, dmnsn_color light)
{
  if (state->pigment.T >= dmnsn_epsilon
      && (state->scene->quality & DMNSN_RENDER_TRANSPARENCY))
  {
    return dmnsn_tcolor_filter(light, state->pigment);
  } else {
    return dmnsn_black;
  }
}

/// Get a light's diffuse contribution to the object
static inline dmnsn_color
dmnsn_evaluate_diffuse(const dmnsn_rtstate *state)
{
  const dmnsn_finish *finish = &state->texture->finish;
  if (finish->diffuse) {
    return finish->diffuse->diffuse_fn(
      finish->diffuse, state->light_color, state->pigment.c,
      state->light_ray, state->intersection->normal
    );
  } else {
    return dmnsn_black;
  }
}

/// Get the color of a light ray at an intersection point.
static bool
dmnsn_trace_light_ray(dmnsn_rtstate *state, const dmnsn_light *light)
{
  dmnsn_ray shadow_ray = dmnsn_new_ray(
    state->r,
    light->direction_fn(light, state->r)
  );
  // Add epsilon to avoid hitting ourselves with the shadow ray
  shadow_ray = dmnsn_ray_add_epsilon(shadow_ray);

  // Check if we're casting a shadow on ourself
  if ((dmnsn_vector_dot(shadow_ray.n, state->intersection->normal)
       * dmnsn_vector_dot(state->viewer, state->intersection->normal) < 0.0)
      && (!state->is_shadow_ray || state->pigment.T < dmnsn_epsilon))
  {
    return false;
  }

  state->light_ray = dmnsn_vector_normalized(shadow_ray.n);
  state->light_color = light->illumination_fn(light, state->r);

  // Test for shadow ray intersections
  dmnsn_intersection shadow_caster;
  bool in_shadow = dmnsn_bvh_intersection(state->bvh, shadow_ray,
                                          &shadow_caster, false);
  if (!in_shadow || !light->shadow_fn(light, shadow_caster.t)) {
    return true;
  }

  if (state->reclevel > 0
      && dmnsn_color_intensity(state->adc_value) >= state->scene->adc_bailout
      && (state->scene->quality & DMNSN_RENDER_TRANSPARENCY)) {
    dmnsn_rtstate shadow_state = *state;
    dmnsn_rtstate_initialize(&shadow_state, &shadow_caster);
    dmnsn_trace_pigment(&shadow_state);

    if (shadow_state.pigment.T >= dmnsn_epsilon) {
      --shadow_state.reclevel;
      shadow_state.adc_value = dmnsn_evaluate_transparency(
        &shadow_state, shadow_state.adc_value
      );
      shadow_state.is_shadow_ray = true;
      if (dmnsn_trace_light_ray(&shadow_state, light)) {
        state->light_color = shadow_state.light_color;

        // Handle reflection
        dmnsn_color reflected = dmnsn_evaluate_reflection(
          &shadow_state, state->light_color, state->light_ray
        );
        state->light_color = dmnsn_color_sub(state->light_color, reflected);

        // Handle transparency
        state->light_color = dmnsn_evaluate_transparency(
          &shadow_state, state->light_color
        );

        return true;
      }
    }
  }

  return false;
}

static void
dmnsn_trace_lighting(dmnsn_rtstate *state)
{
  // Calculate the ambient color
  state->color = DMNSN_TCOLOR(dmnsn_black);
  const dmnsn_finish *finish = &state->texture->finish;
  if (finish->ambient) {
    dmnsn_color ambient = finish->ambient->ambient;

    // Handle reflection and transmittance of the ambient light
    dmnsn_color reflected = dmnsn_evaluate_reflection(
      state, ambient, state->intersection->normal
    );
    ambient = dmnsn_color_sub(ambient, reflected);
    dmnsn_color transmitted = dmnsn_evaluate_transparency(state, ambient);
    ambient = dmnsn_color_sub(ambient, transmitted);

    state->color.c = dmnsn_color_illuminate(ambient, state->pigment.c);
  }

  // Iterate over each light
  DMNSN_ARRAY_FOREACH (dmnsn_light **, light, state->scene->lights) {
    if (dmnsn_trace_light_ray(state, *light)) {
      if (state->scene->quality & DMNSN_RENDER_FINISH) {
        dmnsn_color specular = dmnsn_evaluate_specular(state);
        state->light_color = dmnsn_color_sub(state->light_color, specular);

        dmnsn_color reflected = dmnsn_evaluate_reflection(
          state, state->light_color, state->reflected
        );
        state->light_color = dmnsn_color_sub(state->light_color, reflected);

        dmnsn_color transmitted = dmnsn_evaluate_transparency(
          state, state->light_color
        );
        state->light_color = dmnsn_color_sub(state->light_color, transmitted);

        dmnsn_color diffuse = dmnsn_evaluate_diffuse(state);

        state->color.c = dmnsn_color_add(state->color.c, specular);
        state->color.c = dmnsn_color_add(state->color.c, diffuse);
      } else {
        state->color.c = state->pigment.c;
        break;
      }
    }
  }
}

static void
dmnsn_trace_reflection(dmnsn_rtstate *state)
{
  const dmnsn_reflection *reflection = state->texture->finish.reflection;
  if (reflection) {
    dmnsn_ray refl_ray = dmnsn_new_ray(state->r, state->reflected);
    refl_ray = dmnsn_ray_add_epsilon(refl_ray);

    dmnsn_rtstate recursive_state = *state;

    // Calculate ADC value
    recursive_state.adc_value = dmnsn_evaluate_reflection(
      state, state->adc_value, state->reflected
    );

    // Shoot the reflected ray
    dmnsn_color rec = dmnsn_ray_shoot(&recursive_state, refl_ray).c;
    dmnsn_color reflected = dmnsn_evaluate_reflection(
      state, rec, state->reflected
    );

    state->color.c = dmnsn_color_add(state->color.c, reflected);
  }
}

static void
dmnsn_trace_transparency(dmnsn_rtstate *state)
{
  if (state->pigment.T >= dmnsn_epsilon) {
    const dmnsn_interior *interior = state->interior;

    dmnsn_ray trans_ray = dmnsn_new_ray(state->r, state->intersection->ray.n);
    trans_ray = dmnsn_ray_add_epsilon(trans_ray);

    dmnsn_vector r = dmnsn_vector_normalized(trans_ray.n);
    dmnsn_vector n = state->intersection->normal;

    dmnsn_rtstate recursive_state = *state;

    // Calculate new refractive index
    if (dmnsn_vector_dot(r, n) < 0.0) {
      // We are entering an object
      recursive_state.ior = interior->ior;
      recursive_state.parent = state;
    } else {
      // We are leaving an object
      recursive_state.ior = state->parent ? state->parent->ior : 1.0;
      recursive_state.parent = state->parent ? state->parent->parent : NULL;
    }

    // Calculate transmitted ray direction
    double iorr = state->ior/recursive_state.ior; // ior ratio
    double c1 = -dmnsn_vector_dot(r, n);
    double c2 = 1.0 - iorr*iorr*(1.0 - c1*c1);
    if (c2 <= 0.0) {
      // Total internal reflection
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

    // Calculate ADC value
    recursive_state.adc_value = dmnsn_evaluate_transparency(
      state, state->adc_value
    );
    dmnsn_color adc_reflected = dmnsn_evaluate_reflection(
      state, recursive_state.adc_value, state->reflected
    );
    recursive_state.adc_value = dmnsn_color_sub(
      recursive_state.adc_value, adc_reflected
    );

    // Shoot the transmitted ray
    dmnsn_color rec = dmnsn_ray_shoot(&recursive_state, trans_ray).c;
    dmnsn_color filtered = dmnsn_evaluate_transparency(state, rec);

    // Conserve energy
    dmnsn_color reflected = dmnsn_evaluate_reflection(
      state, filtered, state->reflected
    );
    filtered = dmnsn_color_sub(filtered, reflected);

    state->color.c = dmnsn_color_add(state->color.c, filtered);
  }
}
