/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * Dimension is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU Lesser General Public License as published *
 * by the Free Software Foundation; either version 3 of the License, or  *
 * (at your option) any later version.                                   *
 *                                                                       *
 * Dimension is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include "dimension.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
  FILE *file;
  dmnsn_scene *scene;
  dmnsn_object *sphere;
  dmnsn_sRGB sRGB;
  dmnsn_color color;

  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  scene = dmnsn_new_scene(768, 480);

  sRGB.R = 0.0;
  sRGB.G = 0.0;
  sRGB.B = 0.1;
  color = dmnsn_color_from_sRGB(sRGB);
  color.filter = 0.1;
  scene->background = color;

  sphere = dmnsn_new_sphere();
  dmnsn_array_push(scene->objects, &sphere);

  dmnsn_raytrace_scene(scene);

  file = fopen("raytrace.png", "wb");
  dmnsn_png_write_canvas(scene->canvas, file);

  return EXIT_SUCCESS;
}
