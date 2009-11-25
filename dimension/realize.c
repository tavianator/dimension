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

static double
dmnsn_realize_float(dmnsn_astnode astnode)
{
  switch (astnode.type) {
  case DMNSN_AST_FLOAT:
    return *(double *)astnode.ptr;
  case DMNSN_AST_INTEGER:
    return *(long *)astnode.ptr;

  default:
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Invalid float.");
    return 0; /* Silence compiler warning */
  }
}

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

  double x = dmnsn_realize_float(xnode),
         y = dmnsn_realize_float(ynode),
         z = dmnsn_realize_float(znode);

  return dmnsn_new_vector(x, y, z);
}

static dmnsn_object *
dmnsn_realize_rotation(dmnsn_astnode astnode, dmnsn_object *object)
{
  const double deg2rad = atan(1.0)/45.0;

  dmnsn_astnode angle_node;
  dmnsn_array_get(astnode.children, 0, &angle_node);

  dmnsn_vector angle = dmnsn_vector_mul(
    deg2rad,
    dmnsn_realize_vector(angle_node)
  );

  /* Rotations in POV-Ray are done about the X-axis first, then Y, then Z */
  object->trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(angle.x, 0.0, 0.0)),
    object->trans
  );
  object->trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(0.0, angle.y, 0.0)),
    object->trans
  );
  object->trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(0.0, 0.0, angle.z)),
    object->trans
  );

  return object;
}

static dmnsn_object *
dmnsn_realize_object_modifiers(dmnsn_astnode astnode, dmnsn_object *object)
{
  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode modifier;
    dmnsn_array_get(astnode.children, i, &modifier);

    switch (modifier.type) {
    case DMNSN_AST_ROTATION:
      object = dmnsn_realize_rotation(modifier, object);
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Invalid object modifier.");
    }
  }

  return object;
}

static dmnsn_object *
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
  if (!box) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate box.");
  }

  box->trans = dmnsn_scale_matrix(
    dmnsn_new_vector(fabs(x2.x - x1.x)/2.0,
                     fabs(x2.y - x1.y)/2.0,
                     fabs(x2.z - x1.z)/2.0)
  );
  box->trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(dmnsn_new_vector((x2.x + x1.x)/2.0,
                                              (x2.y + x1.y)/2.0,
                                              (x2.z + x1.z)/2.0)),
    box->trans
  );

  dmnsn_astnode modifiers;
  dmnsn_array_get(astnode.children, 2, &modifiers);
  box = dmnsn_realize_object_modifiers(modifiers, box);

  return box;
}

static dmnsn_object *
dmnsn_realize_sphere(dmnsn_astnode astnode)
{
  if (astnode.type != DMNSN_AST_SPHERE) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a sphere.");
  }

  dmnsn_astnode center, radius;
  dmnsn_array_get(astnode.children, 0, &center);
  dmnsn_array_get(astnode.children, 1, &radius);

  dmnsn_vector x0 = dmnsn_realize_vector(center);
  double r = dmnsn_realize_float(radius);

  dmnsn_object *sphere = dmnsn_new_sphere();
  if (!sphere) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate sphere.");
  }

  sphere->texture = dmnsn_new_texture();
  if (!sphere->texture) {
    dmnsn_delete_object(sphere);
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate sphere texture.");
  }

  sphere->texture->pigment = dmnsn_new_solid_pigment(dmnsn_white);
  if (!sphere->texture->pigment) {
    dmnsn_delete_object(sphere);
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate sphere pigment.");
  }

  sphere->trans = dmnsn_scale_matrix(dmnsn_new_vector(r, r, r));
  sphere->trans = dmnsn_matrix_mul(dmnsn_translation_matrix(x0), sphere->trans);

  dmnsn_astnode modifiers;
  dmnsn_array_get(astnode.children, 2, &modifiers);
  sphere = dmnsn_realize_object_modifiers(modifiers, sphere);

  return sphere;
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
    dmnsn_delete_scene(scene);
    return NULL;
  }

  /* Set up the transformation matrix for the perspective camera */
  dmnsn_matrix trans = dmnsn_scale_matrix(
    dmnsn_new_vector(
      ((double)scene->canvas->x)/scene->canvas->y, 1.0, 1.0
    )
  );
  trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(dmnsn_new_vector(0.0, 0.0, -4.0)),
    trans
  );
  trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(0.0, 1.0, 0.0)),
    trans
  );

  /* Create a perspective camera */
  scene->camera = dmnsn_new_perspective_camera();
  if (!scene->camera) {
    dmnsn_delete_scene(scene);
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

    case DMNSN_AST_SPHERE:
      object = dmnsn_realize_sphere(astnode);
      dmnsn_array_push(scene->objects, &object);
      break;

    default:
      fprintf(stderr, "Unrecognised syntax element '%s'.\n",
              dmnsn_astnode_string(astnode.type));
      dmnsn_delete_scene(scene);
      return NULL;
    }
  }

  return scene;
}
