/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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
    scene->default_texture = NULL;

    scene->camera  = NULL;
    scene->canvas  = NULL;
    scene->objects = dmnsn_new_array(sizeof(dmnsn_object *));
    scene->lights  = dmnsn_new_array(sizeof(dmnsn_light *));
    scene->quality = DMNSN_RENDER_FULL;
  }
  return scene;
}

/* Free a scene */
void
dmnsn_delete_scene(dmnsn_scene *scene)
{
  if (scene) {
    dmnsn_delete_array(scene->lights);
    dmnsn_delete_array(scene->objects);
    free(scene);
  }
}
