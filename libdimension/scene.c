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
#include <stdlib.h>
#include <unistd.h> /* For sysconf */

/* Allocate an empty scene */
dmnsn_scene *
dmnsn_new_scene()
{
  dmnsn_scene *scene = dmnsn_malloc(sizeof(dmnsn_scene));

  scene->default_texture = dmnsn_new_texture();
  scene->camera          = NULL;
  scene->canvas          = NULL;
  scene->objects         = dmnsn_new_array(sizeof(dmnsn_object *));
  scene->lights          = dmnsn_new_array(sizeof(dmnsn_light *));
  scene->quality         = DMNSN_RENDER_FULL;
  scene->reclimit        = 5;

  /* Find the number of processors/cores running (TODO: do this portably) */
  int nprocs = sysconf(_SC_NPROCESSORS_ONLN);
  if (nprocs < 1)
    nprocs = 1;
  scene->nthreads = nprocs;

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
    dmnsn_delete_texture(scene->default_texture);
    free(scene);
  }
}
