/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@tavianator.com>          *
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
 * Entire scenes.
 */

#ifndef DIMENSION_SCENE_H
#define DIMENSION_SCENE_H

/** Render quality flags. */
enum {
  DMNSN_RENDER_NONE         = 0,      /**< Render nothing. */
  DMNSN_RENDER_PIGMENT      = 1 << 0, /**< Render pigments. */
  DMNSN_RENDER_LIGHTS       = 1 << 1, /**< Render lights and shadows. */
  DMNSN_RENDER_FINISH       = 1 << 2, /**< Render object finishes. */
  DMNSN_RENDER_TRANSLUCENCY = 1 << 3, /**< Render translucency/refraction. */
  DMNSN_RENDER_REFLECTION   = 1 << 4, /**< Render specular reflection. */
  DMNSN_RENDER_FULL         = ~DMNSN_RENDER_NONE /**< Render everything. */
};

/** Render quality. */
typedef unsigned int dmnsn_quality;

/** An entire scene. */
typedef struct dmnsn_scene {
  /* World attributes */
  dmnsn_color background;         /**< Background color. */
  dmnsn_color ambient;            /**< Global ambient color. */
  dmnsn_sky_sphere *sky_sphere;   /**< Sky sphere. */
  dmnsn_texture *default_texture; /**< Default object texture. */

  /** Camera. */
  dmnsn_camera *camera;

  /** Canvas. */
  dmnsn_canvas *canvas;

  /** Objects. */
  dmnsn_array *objects;

  /** Lights. */
  dmnsn_array *lights;

  /** Render quality. */
  dmnsn_quality quality;

  /** Recursion limit. */
  unsigned int reclimit;

  /** Adaptive depth control bailout. */
  double adc_bailout;

  /** Number of parallel threads. */
  unsigned int nthreads;

  /** Timers. */
  dmnsn_timer *bounding_timer;
  dmnsn_timer *render_timer;
} dmnsn_scene;

/**
 * Create a scene.
 * @return A new empty scene.
 */
dmnsn_scene *dmnsn_new_scene(void);

/**
 * Delete a scene.
 * @param[in,out] scene  The scene to delete.
 */
void dmnsn_delete_scene(dmnsn_scene *scene);

/**
 * Initialize a scene.
 * @param[in,out] scene  The scene to initalize.
 */
void dmnsn_initialize_scene(dmnsn_scene *scene);

#endif /* DIMENSION_SCENE_H */
