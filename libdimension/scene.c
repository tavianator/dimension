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

#include "dimension.h"
#include <stdlib.h> /* For malloc */

/* Allocate an empty scene */
dmnsn_scene *
dmnsn_new_scene()
{
  dmnsn_scene *scene = malloc(sizeof(dmnsn_scene));
  if (scene) {
    scene->default_texture = dmnsn_new_texture();
    if (!scene->default_texture) {
      dmnsn_delete_scene(scene);
      return NULL;
    }

    scene->camera   = NULL;
    scene->canvas   = NULL;
    scene->objects  = dmnsn_new_array(sizeof(dmnsn_object *));
    scene->lights   = dmnsn_new_array(sizeof(dmnsn_light *));
    scene->quality  = DMNSN_RENDER_FULL;
    scene->reclimit = 5;
    scene->nthreads = 0;
  }
  return scene;
}

/* Free a scene */
void
dmnsn_delete_scene(dmnsn_scene *scene)
{
  if (scene) {
    unsigned int i;
    dmnsn_light *light;
    dmnsn_object *object;

    for (i = 0; i < dmnsn_array_size(scene->lights); ++i) {
      dmnsn_array_get(scene->lights, i, &light);
      dmnsn_delete_light(light);
    }

    for (i = 0; i < dmnsn_array_size(scene->objects); ++i) {
      dmnsn_array_get(scene->objects, i, &object);
      dmnsn_delete_object(object);
    }

    dmnsn_delete_array(scene->lights);
    dmnsn_delete_array(scene->objects);
    dmnsn_delete_canvas(scene->canvas);
    dmnsn_delete_camera(scene->camera);
    dmnsn_delete_texture(scene->default_texture);
    free(scene);
  }
}
