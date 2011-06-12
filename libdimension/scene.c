/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "dimension-impl.h"
#include <stdlib.h>

/* Allocate an empty scene */
dmnsn_scene *
dmnsn_new_scene(void)
{
  dmnsn_scene *scene = dmnsn_malloc(sizeof(dmnsn_scene));

  scene->background       = dmnsn_black;
  scene->sky_sphere       = NULL;
  scene->default_texture  = dmnsn_new_texture();
  scene->default_interior = dmnsn_new_interior();
  scene->canvas           = NULL;
  scene->objects          = dmnsn_new_array(sizeof(dmnsn_object *));
  scene->lights           = dmnsn_new_array(sizeof(dmnsn_light *));
  scene->camera           = NULL;
  scene->quality          = DMNSN_RENDER_FULL;
  scene->reclimit         = 5;
  scene->adc_bailout      = 1.0/255.0;
  scene->nthreads         = dmnsn_ncpus();
  scene->bounding_timer   = NULL;
  scene->render_timer     = NULL;
  scene->initialized      = false;

  return scene;
}

/* Free a scene */
void
dmnsn_delete_scene(dmnsn_scene *scene)
{
  if (scene) {
    dmnsn_delete_timer(scene->render_timer);
    dmnsn_delete_timer(scene->bounding_timer);

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
    dmnsn_delete_sky_sphere(scene->sky_sphere);
    dmnsn_free(scene);
  }
}

void
dmnsn_initialize_scene(dmnsn_scene *scene)
{
  dmnsn_assert(!scene->initialized, "Texture double-initialized.");
  scene->initialized = true;

  dmnsn_initialize_texture(scene->default_texture);

  if (scene->sky_sphere) {
    dmnsn_initialize_sky_sphere(scene->sky_sphere);
  }

  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, scene->objects) {
    dmnsn_texture_cascade(scene->default_texture, &(*object)->texture);
    dmnsn_interior_cascade(scene->default_interior, &(*object)->interior);
    dmnsn_initialize_object(*object);
  }
}
