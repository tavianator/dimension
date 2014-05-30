/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Scenes.
 */

#include "dimension-internal.h"
#include <stdlib.h>

/* Allocate an empty scene */
dmnsn_scene *
dmnsn_new_scene(dmnsn_pool *pool)
{
  dmnsn_scene *scene = DMNSN_PALLOC(pool, dmnsn_scene);

  scene->background       = NULL;
  scene->default_texture  = dmnsn_new_texture();
  scene->default_interior = dmnsn_new_interior();
  scene->canvas           = NULL;
  scene->region_x         = 0;
  scene->region_y         = 0;
  scene->outer_width      = 0;
  scene->outer_height     = 0;
  scene->objects          = dmnsn_new_array(sizeof(dmnsn_object *));
  scene->lights           = dmnsn_new_array(sizeof(dmnsn_light *));
  scene->camera           = NULL;
  scene->quality          = DMNSN_RENDER_FULL;
  scene->reclimit         = 5;
  scene->adc_bailout      = 1.0/255.0;
  scene->nthreads         = dmnsn_ncpus();
  scene->initialized      = false;

  return scene;
}

/* Free a scene */
void
dmnsn_delete_scene(dmnsn_scene *scene)
{
  if (scene) {
    DMNSN_ARRAY_FOREACH (dmnsn_light **, light, scene->lights) {
      dmnsn_delete_light(*light);
    }
    DMNSN_ARRAY_FOREACH (dmnsn_object **, object, scene->objects) {
      dmnsn_delete_object(*object);
    }

    dmnsn_delete_array(scene->lights);
    dmnsn_delete_array(scene->objects);
    dmnsn_delete_canvas(scene->canvas);
    dmnsn_delete_camera(scene->camera);
    dmnsn_delete_interior(scene->default_interior);
    dmnsn_delete_texture(scene->default_texture);
    dmnsn_delete_pigment(scene->background);
  }
}

void
dmnsn_scene_initialize(dmnsn_scene *scene)
{
  dmnsn_assert(!scene->initialized, "Scene double-initialized.");
  scene->initialized = true;

  if (scene->outer_width == 0) {
    scene->outer_width = scene->canvas->width;
  }
  if (scene->outer_height == 0) {
    scene->outer_height = scene->canvas->height;
  }

  dmnsn_pigment_initialize(scene->background);

  dmnsn_texture_initialize(scene->default_texture);

  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, scene->objects) {
    dmnsn_texture_cascade(scene->default_texture, &(*object)->texture);
    dmnsn_interior_cascade(scene->default_interior, &(*object)->interior);
    dmnsn_object_initialize(*object);
  }
}
