/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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

dmnsn_scene *
dmnsn_new_scene()
{
  dmnsn_scene *scene = malloc(sizeof(dmnsn_scene));
  if (scene)
    scene->objects = dmnsn_new_array(sizeof(dmnsn_object*));
  return scene;
}

void
dmnsn_delete_scene(dmnsn_scene *scene)
{
  if (scene) {
    dmnsn_delete_array(scene->objects);
    free(scene);
  }
}

void
dmnsn_raytrace_scene(dmnsn_scene *scene)
{
  unsigned int i, j;
  dmnsn_object *object;
  dmnsn_line ray;

  dmnsn_array_get(scene->objects, 0, &object);

  for (i = 0; i < scene->canvas->x; ++i) {
    for (j = 0; j < scene->canvas->y; ++j) {
      ray = (*scene->camera->ray_fn)(scene->camera, scene->canvas, i, j);
      if ((*object->intersections_fn)(object, ray)->length > 0) {
        dmnsn_set_pixel(scene->canvas, i, j,
                        dmnsn_color_from_XYZ(dmnsn_whitepoint));
      } else {
        dmnsn_set_pixel(scene->canvas, i, j, scene->background);
      }
    }
  }
}
