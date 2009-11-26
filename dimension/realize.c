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

static dmnsn_color
dmnsn_realize_color(dmnsn_astnode astnode)
{
  if (astnode.type != DMNSN_AST_VECTOR) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a color.");
  }


  dmnsn_astnode rnode, gnode, bnode, fnode, tnode;
  dmnsn_array_get(astnode.children, 0, &rnode);
  dmnsn_array_get(astnode.children, 1, &gnode);
  dmnsn_array_get(astnode.children, 2, &bnode);
  dmnsn_array_get(astnode.children, 3, &fnode);
  dmnsn_array_get(astnode.children, 4, &tnode);

  double r = dmnsn_realize_float(rnode),
         g = dmnsn_realize_float(gnode),
         b = dmnsn_realize_float(bnode),
         f = dmnsn_realize_float(fnode),
         t = dmnsn_realize_float(tnode);

  dmnsn_sRGB sRGB = { .R = r, .G = g, .B = b };
  dmnsn_color color = dmnsn_color_from_sRGB(sRGB);
  color.filter = f;
  color.trans  = t;

  return color;
}

static dmnsn_object *
dmnsn_realize_rotation(dmnsn_astnode astnode, dmnsn_object *object)
{
  if (astnode.type != DMNSN_AST_ROTATION) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a rotation.");
  }

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
dmnsn_realize_scale(dmnsn_astnode astnode, dmnsn_object *object)
{
  if (astnode.type != DMNSN_AST_SCALE) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a scale.");
  }

  dmnsn_astnode scale_node;
  dmnsn_array_get(astnode.children, 0, &scale_node);
  dmnsn_vector scale = dmnsn_realize_vector(scale_node);

  object->trans = dmnsn_matrix_mul(dmnsn_scale_matrix(scale), object->trans);
  return object;
}

static dmnsn_object *
dmnsn_realize_translation(dmnsn_astnode astnode, dmnsn_object *object)
{
  if (astnode.type != DMNSN_AST_TRANSLATION) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a translation.");
  }

  dmnsn_astnode trans_node;
  dmnsn_array_get(astnode.children, 0, &trans_node);
  dmnsn_vector trans = dmnsn_realize_vector(trans_node);

  object->trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(trans),
    object->trans
  );
  return object;
}

static dmnsn_object *
dmnsn_realize_pigment(dmnsn_astnode astnode, dmnsn_object *object)
{
  if (astnode.type != DMNSN_AST_PIGMENT) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a pigment.");
  }

  if (!object->texture) {
    object->texture = dmnsn_new_texture();
    if (!object->texture) {
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't create texture.");
    }
  }
  dmnsn_delete_pigment(object->texture->pigment);

  dmnsn_astnode color_node;
  dmnsn_array_get(astnode.children, 0, &color_node);

  dmnsn_color color;
  switch (color_node.type) {
  case DMNSN_AST_NONE:
    break;

  case DMNSN_AST_VECTOR:
    color = dmnsn_realize_color(color_node);
    object->texture->pigment = dmnsn_new_solid_pigment(color);
    if (!object->texture->pigment) {
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't create pigment.");
    }
    break;

  default:
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Invalid pigment color.");
  }

  return object;
}

static dmnsn_object *
dmnsn_realize_texture(dmnsn_astnode astnode, dmnsn_object *object)
{
  if (astnode.type != DMNSN_AST_TEXTURE) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a texture.");
  }

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode modifier;
    dmnsn_array_get(astnode.children, i, &modifier);

    switch (modifier.type) {
    case DMNSN_AST_PIGMENT:
      object = dmnsn_realize_pigment(modifier, object);
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Invalid texture item.");
    }
  }

  return object;
}

static dmnsn_object *
dmnsn_realize_object_modifiers(dmnsn_astnode astnode, dmnsn_object *object)
{
  if (astnode.type != DMNSN_AST_OBJECT_MODIFIERS) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected object modifiers.");
  }

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode modifier;
    dmnsn_array_get(astnode.children, i, &modifier);

    switch (modifier.type) {
    case DMNSN_AST_ROTATION:
      object = dmnsn_realize_rotation(modifier, object);
      break;
    case DMNSN_AST_SCALE:
      object = dmnsn_realize_scale(modifier, object);
      break;
    case DMNSN_AST_TRANSLATION:
      object = dmnsn_realize_translation(modifier, object);
      break;

    case DMNSN_AST_TEXTURE:
      object = dmnsn_realize_texture(modifier, object);
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

static dmnsn_light *
dmnsn_realize_light_source(dmnsn_astnode astnode)
{
  if (astnode.type != DMNSN_AST_LIGHT_SOURCE) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Expected a light source.");
  }

  dmnsn_astnode point, color_node;
  dmnsn_array_get(astnode.children, 0, &point);
  dmnsn_array_get(astnode.children, 1, &color_node);

  dmnsn_vector x0 = dmnsn_realize_vector(point);
  dmnsn_color color = dmnsn_realize_color(color_node);

  dmnsn_light *light = dmnsn_new_point_light(x0, color);
  if (!light) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate light.");
  }

  return light;
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

  /* Default finish */
  scene->default_texture->finish = dmnsn_new_phong_finish(1.0, 0.0, 1.0);
  if (!scene->default_texture->finish) {
    dmnsn_delete_scene(scene);
    return NULL;
  }
  scene->default_texture->finish->ambient = 0.1;

  /* Background color */
  scene->background = dmnsn_black;

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

    dmnsn_light  *light;
    dmnsn_object *object;
    switch (astnode.type) {
    case DMNSN_AST_BACKGROUND:
      dmnsn_array_get(astnode.children, 0, &astnode);
      scene->background = dmnsn_realize_color(astnode);
      break;

    case DMNSN_AST_BOX:
      object = dmnsn_realize_box(astnode);
      dmnsn_array_push(scene->objects, &object);
      break;

    case DMNSN_AST_LIGHT_SOURCE:
      light = dmnsn_realize_light_source(astnode);
      dmnsn_array_push(scene->lights, &light);
      break;

    case DMNSN_AST_SPHERE:
      object = dmnsn_realize_sphere(astnode);
      dmnsn_array_push(scene->objects, &object);
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Unrecognised syntax element.");
    }
  }

  return scene;
}
