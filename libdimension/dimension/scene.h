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

/*
 * A scene.
 */

#ifndef DIMENSION_SCENE_H
#define DIMENSION_SCENE_H

typedef enum {
  DMNSN_RENDER_NONE         = 0,
  DMNSN_RENDER_PIGMENT      = 1 << 0,
  DMNSN_RENDER_LIGHTS       = 1 << 1,
  DMNSN_RENDER_FINISH       = 1 << 2,
  DMNSN_RENDER_TRANSLUCENCY = 1 << 3,
  DMNSN_RENDER_FULL         = ~DMNSN_RENDER_NONE
} dmnsn_quality;

typedef struct {
  /* World attributes */
  dmnsn_color background;
  dmnsn_texture *default_texture;

  /* Camera */
  dmnsn_camera *camera;

  /* Canvas */
  dmnsn_canvas *canvas;

  /* Objects */
  dmnsn_array *objects;

  /* Lights */
  dmnsn_array *lights;

  /* Rendering quality */
  dmnsn_quality quality;
} dmnsn_scene;

/* Create a scene */
dmnsn_scene *dmnsn_new_scene();
void dmnsn_delete_scene(dmnsn_scene *scene);

#endif /* DIMENSION_SCENE_H */
