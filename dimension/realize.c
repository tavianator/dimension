/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of Dimension.                                       *
 *                                                                       *
 * Dimension is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU General Public License as published by the *
 * Free Software Foundation; either version 3 of the License, or (at     *
 * your option) any later version.                                       *
 *                                                                       *
 * Dimension is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "realize.h"
#include "parse.h"
#include "utility.h"
#include <math.h>

static dmnsn_vector
dmnsn_realize_vector(dmnsn_astnode astnode)
{
  if (astnode.type != DMNSN_AST_VECTOR) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a vector.");
  }

  dmnsn_astnode xnode, ynode, znode;
  dmnsn_array_get(astnode.children, 0, &xnode);
  dmnsn_array_get(astnode.children, 1, &ynode);
  dmnsn_array_get(astnode.children, 2, &znode);

  if (xnode.type != DMNSN_AST_FLOAT
      || ynode.type != DMNSN_AST_FLOAT
      || znode.type != DMNSN_AST_FLOAT)
  {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a float.");
  }

  double *x = xnode.ptr, *y = ynode.ptr, *z = znode.ptr;
  return dmnsn_vector_construct(*x, *y, *z);
}

dmnsn_object *
dmnsn_realize_box(dmnsn_astnode astnode)
{
  if (astnode.type != DMNSN_AST_BOX) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a box.");
  }

  dmnsn_astnode corner1, corner2;
  dmnsn_array_get(astnode.children, 0, &corner1);
  dmnsn_array_get(astnode.children, 1, &corner2);

  dmnsn_vector x1 = dmnsn_realize_vector(corner1),
               x2 = dmnsn_realize_vector(corner2);

  dmnsn_object *box = dmnsn_new_cube();
  box->trans = dmnsn_scale_matrix(
    dmnsn_vector_construct(fabs(x2.x - x1.x)/2.0,
                           fabs(x2.y - x1.y)/2.0,
                           fabs(x2.z - x1.z)/2.0)
  );
  box->trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(dmnsn_vector_construct((x2.x + x1.x)/2.0,
                                                    (x2.y + x1.y)/2.0,
                                                    (x2.z + x1.z)/2.0)),
    box->trans
  );
  box->trans = dmnsn_matrix_inverse(box->trans);

  return box;
}

dmnsn_scene *
dmnsn_realize(const dmnsn_array *astree)
{
  dmnsn_scene *scene = dmnsn_new_scene();
  if (!scene) {
    return NULL;
  }

  /* Background color */
  dmnsn_sRGB background_sRGB = { .R = 0.0, .G = 0.0, .B = 0.1 };
  scene->background = dmnsn_color_from_sRGB(background_sRGB);
  scene->background.filter = 0.1;

  /* Allocate a canvas */
  scene->canvas = dmnsn_new_canvas(768, 480);
  if (!scene->canvas) {
    dmnsn_delete_realized_scene(scene);
    return NULL;
  }

  /* Set up the transformation matrix for the perspective camera */
  dmnsn_matrix trans = dmnsn_scale_matrix(
    dmnsn_vector_construct(
      ((double)scene->canvas->x)/scene->canvas->y, 1.0, 1.0
    )
  );
  trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(dmnsn_vector_construct(0.0, 0.0, -4.0)),
    trans
  );
  trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_vector_construct(0.8, 0.7, 0.0)),
    trans
  );

  /* Create a perspective camera */
  scene->camera = dmnsn_new_perspective_camera();
  if (!scene->camera) {
    dmnsn_delete_realized_scene(scene);
    return NULL;
  }
  dmnsn_set_perspective_camera_trans(scene->camera, trans);

  /*
   * Now parse the abstract syntax tree
   */

  dmnsn_astnode astnode;
  unsigned int i;

  for (i = 0; i < dmnsn_array_size(astree); ++i) {
    dmnsn_array_get(astree, i, &astnode);

    dmnsn_object *object;
    switch (astnode.type) {
    case DMNSN_AST_BOX:
      object = dmnsn_realize_box(astnode);
      dmnsn_array_push(scene->objects, &object);
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH, "We only handle boxes right now.");
    }
  }

  return scene;
}

void
dmnsn_delete_realized_scene(dmnsn_scene *scene)
{
  dmnsn_object *object;
  unsigned int i;

  for (i = 0; i < dmnsn_array_size(scene->objects); ++i) {
    dmnsn_array_get(scene->objects, i, &object);
    dmnsn_delete_object(object);
  }

  dmnsn_delete_camera(scene->camera);
  dmnsn_delete_canvas(scene->canvas);
  dmnsn_delete_scene(scene);
}
