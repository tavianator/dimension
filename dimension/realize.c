/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
#include <stdio.h>
#include <stdbool.h>

static long
dmnsn_realize_integer(dmnsn_astnode astnode)
{
  switch (astnode.type) {
  case DMNSN_AST_INTEGER:
    return *(long *)astnode.ptr;
  case DMNSN_AST_FLOAT:
    dmnsn_diagnostic(astnode.location, "WARNING: float rounded to integer");
    return *(double *)astnode.ptr;

  default:
    dmnsn_assert(false, "Invalid integer.");
    return 0; /* Silence compiler warning */
  }
}

static double
dmnsn_realize_float(dmnsn_astnode astnode)
{
  switch (astnode.type) {
  case DMNSN_AST_FLOAT:
    return *(double *)astnode.ptr;
  case DMNSN_AST_INTEGER:
    return *(long *)astnode.ptr;

  default:
    dmnsn_assert(false, "Invalid float.");
    return 0; /* Silence compiler warning */
  }
}

/* dmnsn_realize_string is an API function, so call this dmnsn_realize_str */
static const char*
dmnsn_realize_str(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_STRING, "Expected a string.");
  return astnode.ptr;
}

static dmnsn_vector
dmnsn_realize_vector(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_VECTOR, "Expected a vector.");

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
  dmnsn_assert(astnode.type == DMNSN_AST_VECTOR, "Expected a vector.");

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

static dmnsn_matrix
dmnsn_realize_rotation(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_ROTATION, "Expected a rotation.");

  const double deg2rad = atan(1.0)/45.0;

  dmnsn_astnode angle_node;
  dmnsn_array_get(astnode.children, 0, &angle_node);

  dmnsn_vector angle = dmnsn_vector_mul(
    deg2rad,
    dmnsn_realize_vector(angle_node)
  );

  dmnsn_matrix trans = dmnsn_rotation_matrix(
    dmnsn_new_vector(angle.x, 0.0, 0.0)
  );
  trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(0.0, angle.y, 0.0)),
    trans
  );
  trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(0.0, 0.0, angle.z)),
    trans
  );

  return trans;
}

static dmnsn_matrix
dmnsn_realize_scale(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_SCALE, "Expected a scale.");

  dmnsn_astnode scale_node;
  dmnsn_array_get(astnode.children, 0, &scale_node);
  dmnsn_vector scale = dmnsn_realize_vector(scale_node);

  return dmnsn_scale_matrix(scale);
}

static dmnsn_matrix
dmnsn_realize_translation(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_TRANSLATION,
               "Expected a translation.");

  dmnsn_astnode trans_node;
  dmnsn_array_get(astnode.children, 0, &trans_node);
  dmnsn_vector trans = dmnsn_realize_vector(trans_node);

  return dmnsn_translation_matrix(trans);
}

static dmnsn_matrix
dmnsn_realize_transformation(dmnsn_astnode astnode)
{
  switch (astnode.type) {
  case DMNSN_AST_ROTATION:
    return dmnsn_realize_rotation(astnode);
  case DMNSN_AST_SCALE:
    return dmnsn_realize_scale(astnode);
  case DMNSN_AST_TRANSLATION:
    return dmnsn_realize_translation(astnode);

  default:
    dmnsn_assert(false, "Expected a transformation.");
    return dmnsn_identity_matrix(); // Shut up compiler
  }
}

static void
dmnsn_realize_global_settings(dmnsn_astnode astnode, dmnsn_scene *scene)
{
  dmnsn_assert(astnode.type == DMNSN_AST_GLOBAL_SETTINGS,
               "Expected global settings.");

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode item, child;
    dmnsn_array_get(astnode.children, i, &item);

    switch (item.type) {
    case DMNSN_AST_ASSUMED_GAMMA:
      dmnsn_diagnostic(item.location, "WARNING: assumed_gamma not supported");
      break;

    case DMNSN_AST_MAX_TRACE_LEVEL:
      dmnsn_array_get(item.children, 0, &child);
      scene->reclimit = dmnsn_realize_integer(child);
      break;

    default:
      dmnsn_assert(false, "Invalid global settings item.");
    }
  }
}

static dmnsn_camera *
dmnsn_realize_camera(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_CAMERA, "Expected a camera.");

  const double deg2rad = atan(1.0)/45.0;

  dmnsn_astnode_type camera_type = DMNSN_AST_PERSPECTIVE;
  dmnsn_vector location  = dmnsn_new_vector(0.0, 0.0, 0.0);
  dmnsn_vector direction = dmnsn_new_vector(0.0, 0.0, 1.0);
  dmnsn_vector right     = dmnsn_new_vector(4.0/3.0, 0.0, 0.0);
  dmnsn_vector up        = dmnsn_new_vector(0.0, 1.0, 0.0);
  dmnsn_vector sky       = dmnsn_new_vector(0.0, 1.0, 0.0);
  dmnsn_matrix trans     = dmnsn_identity_matrix();

  dmnsn_camera *camera = NULL;

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode item;
    dmnsn_array_get(astnode.children, i, &item);

    switch (item.type) {
    /* Camera types */
    case DMNSN_AST_PERSPECTIVE:
      camera_type = item.type;
      break;

    /* Camera vectors */
    case DMNSN_AST_LOCATION:
      dmnsn_array_get(item.children, 0, &item);
      location = dmnsn_realize_vector(item);
      break;
    case DMNSN_AST_RIGHT:
      dmnsn_array_get(item.children, 0, &item);
      right = dmnsn_realize_vector(item);
      break;
    case DMNSN_AST_UP:
      dmnsn_array_get(item.children, 0, &item);
      right = dmnsn_realize_vector(item);
      break;
    case DMNSN_AST_SKY:
      dmnsn_array_get(item.children, 0, &item);
      sky = dmnsn_realize_vector(item);
      break;
    case DMNSN_AST_DIRECTION:
      dmnsn_array_get(item.children, 0, &item);
      direction = dmnsn_realize_vector(item);
      break;

    /* Camera modifiers */

    case DMNSN_AST_LOOK_AT:
      {
        dmnsn_array_get(item.children, 0, &item);
        dmnsn_vector look_at = dmnsn_realize_vector(item);

        /* Line the camera up with the sky */

        dmnsn_matrix sky1 = dmnsn_rotation_matrix(
          dmnsn_vector_mul(
            dmnsn_vector_axis_angle(up, sky, direction),
            dmnsn_vector_normalize(direction)
          )
        );
        up    = dmnsn_transform_vector(sky1, up);
        right = dmnsn_transform_vector(sky1, right);

        dmnsn_matrix sky2 = dmnsn_rotation_matrix(
          dmnsn_vector_mul(
            dmnsn_vector_axis_angle(up, sky, right),
            dmnsn_vector_normalize(right)
          )
        );
        up        = dmnsn_transform_vector(sky2, up);
        direction = dmnsn_transform_vector(sky2, direction);

        /* Line up the camera with the look_at */

        look_at = dmnsn_vector_sub(look_at, location);
        dmnsn_matrix look_at1 = dmnsn_rotation_matrix(
          dmnsn_vector_mul(
            dmnsn_vector_axis_angle(direction, look_at, up),
            dmnsn_vector_normalize(up)
          )
        );
        right     = dmnsn_transform_vector(look_at1, right);
        direction = dmnsn_transform_vector(look_at1, direction);

        dmnsn_matrix look_at2 = dmnsn_rotation_matrix(
          dmnsn_vector_mul(
            dmnsn_vector_axis_angle(direction, look_at, right),
            dmnsn_vector_normalize(right)
          )
        );
        up        = dmnsn_transform_vector(look_at2, up);
        direction = dmnsn_transform_vector(look_at2, direction);

        break;
      }

    case DMNSN_AST_ANGLE:
      {
        dmnsn_array_get(item.children, 0, &item);
        double angle = deg2rad*dmnsn_realize_float(item);
        direction = dmnsn_vector_mul(
          0.5*dmnsn_vector_norm(right)/tan(angle/2.0),
          dmnsn_vector_normalize(direction)
        );
        break;
      }

    /* Transformations */
    case DMNSN_AST_ROTATION:
    case DMNSN_AST_SCALE:
    case DMNSN_AST_TRANSLATION:
      trans = dmnsn_matrix_mul(dmnsn_realize_transformation(item), trans);
      break;

    default:
      dmnsn_assert(false, "Invalid camera item.");
      break;
    }
  }

  switch (camera_type) {
  case DMNSN_AST_PERSPECTIVE:
    {
      /* These multiplications are in right-to-left order so that user
         transformations happen after camera alignment */

      trans = dmnsn_matrix_mul(trans, dmnsn_translation_matrix(location));

      /* Align y with `up' */

      dmnsn_matrix align = dmnsn_rotation_matrix(
        dmnsn_vector_mul(
          dmnsn_vector_axis_angle(dmnsn_y, up, dmnsn_z),
          dmnsn_z
        )
      );

      dmnsn_vector x = dmnsn_transform_vector(align, dmnsn_x);
      dmnsn_vector y = dmnsn_transform_vector(align, dmnsn_y);

      align = dmnsn_matrix_mul(
        dmnsn_rotation_matrix(
          dmnsn_vector_mul(
            dmnsn_vector_axis_angle(y, up, x),
            x
          )
        ),
        align
      );

      /* Align x with `right' */

      align = dmnsn_matrix_mul(
        dmnsn_rotation_matrix(
          dmnsn_vector_mul(
            dmnsn_vector_axis_angle(x, right, up),
            dmnsn_vector_normalize(up)
          )
        ),
        align
      );

      trans = dmnsn_matrix_mul(trans, align);

      /* Scale the camera with `up', `right', and `direction' */
      trans = dmnsn_matrix_mul(
        trans,
        dmnsn_scale_matrix(
          dmnsn_new_vector(
            dmnsn_vector_norm(right),
            dmnsn_vector_norm(up),
            dmnsn_vector_norm(direction)
          )
        )
      );

      camera = dmnsn_new_perspective_camera();
      dmnsn_set_perspective_camera_trans(camera, trans);
      break;
    }

  default:
    dmnsn_assert(false, "Unsupported camera type.");
  }

  return camera;
}

static void
dmnsn_realize_pigment_modifiers(dmnsn_astnode astnode, dmnsn_pigment *pigment)
{
  dmnsn_assert(astnode.type == DMNSN_AST_PIGMENT_MODIFIERS,
               "Expected pigment modifiers.");

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode modifier;
    dmnsn_array_get(astnode.children, i, &modifier);

    switch (modifier.type) {
    case DMNSN_AST_ROTATION:
    case DMNSN_AST_SCALE:
    case DMNSN_AST_TRANSLATION:
      pigment->trans = dmnsn_matrix_mul(
        dmnsn_realize_transformation(modifier),
        pigment->trans
      );
      break;

    default:
      dmnsn_assert(false, "Invalid pigment modifier.");
    }
  }
}

static dmnsn_pigment *
dmnsn_realize_pigment(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_PIGMENT, "Expected a pigment.");

  dmnsn_pigment *pigment = NULL;

  dmnsn_astnode type_node;
  dmnsn_array_get(astnode.children, 0, &type_node);

  dmnsn_color color;
  switch (type_node.type) {
  case DMNSN_AST_NONE:
    break;

  case DMNSN_AST_VECTOR:
    color = dmnsn_realize_color(type_node);
    pigment = dmnsn_new_solid_pigment(color);
    break;

  case DMNSN_AST_IMAGE_MAP:
    {
      dmnsn_astnode filetype, strnode;
      dmnsn_array_get(type_node.children, 0, &filetype);
      dmnsn_array_get(type_node.children, 1, &strnode);

      const char *path = dmnsn_realize_str(strnode);
      FILE *file = fopen(path, "rb");
      if (!file) {
        dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't open image file.");
        return NULL;
      }

      dmnsn_canvas *canvas;
      switch (filetype.type) {
      case DMNSN_AST_PNG:
        canvas = dmnsn_png_read_canvas(file);
        if (!canvas) {
          dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Invalid PNG file.");
          return NULL;
        }
        pigment = dmnsn_new_canvas_pigment(canvas);
        break;

      default:
        dmnsn_assert(false, "Invalid image_map type.");
        break;
      }
      break;
    }

  default:
    dmnsn_assert(false, "Invalid pigment type.");
  }

  dmnsn_astnode modifiers;
  dmnsn_array_get(astnode.children, 1, &modifiers);
  dmnsn_realize_pigment_modifiers(modifiers, pigment);

  return pigment;
}

static dmnsn_finish *
dmnsn_realize_reflection(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_REFLECTION, "Expected a reflection.");

  dmnsn_astnode min_node, max_node;
  dmnsn_array_get(astnode.children, 0, &min_node);
  dmnsn_array_get(astnode.children, 1, &max_node);

  dmnsn_color min = dmnsn_realize_color(min_node);
  dmnsn_color max = dmnsn_realize_color(max_node);

  double falloff = 1.0;

  dmnsn_astnode items;
  dmnsn_array_get(astnode.children, 2, &items);

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(items.children); ++i) {
    dmnsn_astnode item, child;
    dmnsn_array_get(items.children, i, &item);

    switch (item.type) {
    case DMNSN_AST_FALLOFF:
      dmnsn_array_get(item.children, 0, &child);
      falloff = dmnsn_realize_float(child);
      break;

    default:
      dmnsn_assert(false, "Invalid reflection item.");
    }
  }

  dmnsn_finish *reflection = dmnsn_new_reflective_finish(min, max, falloff);

  return reflection;
}

static dmnsn_finish *
dmnsn_realize_finish(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_FINISH, "Expected a finish.");

  dmnsn_finish *finish = dmnsn_new_finish();

  dmnsn_color ambient = dmnsn_black;
  bool ambient_set = false;

  double diffuse = 0.0;
  bool diffuse_set = false;

  double phong = 0.0;
  double phong_size = 40.0;

  dmnsn_finish *reflection = NULL;

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode item, child;
    dmnsn_array_get(astnode.children, i, &item);

    switch (item.type) {
    case DMNSN_AST_AMBIENT:
      dmnsn_array_get(item.children, 0, &child);
      ambient = dmnsn_realize_color(child);
      ambient_set = true;
      break;

    case DMNSN_AST_DIFFUSE:
      dmnsn_array_get(item.children, 0, &child);
      diffuse = dmnsn_realize_float(child);
      diffuse_set = true;
      break;

    case DMNSN_AST_PHONG:
      dmnsn_array_get(item.children, 0, &child);
      phong = dmnsn_realize_float(child);
      break;
    case DMNSN_AST_PHONG_SIZE:
      dmnsn_array_get(item.children, 0, &child);
      phong_size = dmnsn_realize_float(child);
      break;

    case DMNSN_AST_REFLECTION:
      dmnsn_delete_finish(reflection);
      reflection = dmnsn_realize_reflection(item);
      break;

    default:
      dmnsn_assert(false, "Invalid finish item.");
    }
  }

  if (ambient_set) {
    finish = dmnsn_new_finish_combination(
      dmnsn_new_ambient_finish(ambient),
      finish
    );
  }

  if (diffuse_set) {
    finish = dmnsn_new_finish_combination(
      dmnsn_new_diffuse_finish(diffuse),
      finish
    );
  }

  if (phong) {
    finish = dmnsn_new_finish_combination(
      dmnsn_new_phong_finish(phong, phong_size),
      finish
    );
  }

  if (reflection) {
    finish = dmnsn_new_finish_combination(reflection, finish);
  }

  return finish;
}

static dmnsn_texture *
dmnsn_realize_texture(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_TEXTURE, "Expected a texture.");

  dmnsn_texture *texture = dmnsn_new_texture();

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode item;
    dmnsn_array_get(astnode.children, i, &item);

    switch (item.type) {
    case DMNSN_AST_PIGMENT:
      dmnsn_delete_pigment(texture->pigment);
      texture->pigment = dmnsn_realize_pigment(item);
      break;

    case DMNSN_AST_FINISH:
      dmnsn_delete_finish(texture->finish);
      texture->finish = dmnsn_realize_finish(item);
      break;

    case DMNSN_AST_ROTATION:
    case DMNSN_AST_SCALE:
    case DMNSN_AST_TRANSLATION:
      texture->trans = dmnsn_matrix_mul(dmnsn_realize_transformation(item),
                                        texture->trans);
      break;

    default:
      dmnsn_assert(false, "Invalid texture item.");
    }
  }

  return texture;
}

static dmnsn_interior *
dmnsn_realize_interior(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_INTERIOR, "Expected a texture.");

  dmnsn_interior *interior = dmnsn_new_interior();

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode item, child;
    dmnsn_array_get(astnode.children, i, &item);

    switch (item.type) {
    case DMNSN_AST_IOR:
      dmnsn_array_get(item.children, 0, &child);
      interior->ior = dmnsn_realize_float(child);
      break;

    default:
      dmnsn_assert(false, "Invalid interior item.");
    }
  }

  return interior;
}

static void
dmnsn_realize_object_modifiers(dmnsn_astnode astnode, dmnsn_object *object)
{
  dmnsn_assert(astnode.type == DMNSN_AST_OBJECT_MODIFIERS,
               "Expected object modifiers.");

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode modifier;
    dmnsn_array_get(astnode.children, i, &modifier);

    switch (modifier.type) {
    case DMNSN_AST_ROTATION:
    case DMNSN_AST_SCALE:
    case DMNSN_AST_TRANSLATION:
      object->trans = dmnsn_matrix_mul(
        dmnsn_realize_transformation(modifier),
        object->trans
      );
      break;

    case DMNSN_AST_TEXTURE:
      dmnsn_delete_texture(object->texture);
      object->texture = dmnsn_realize_texture(modifier);
      break;
    case DMNSN_AST_PIGMENT:
      if (!object->texture)
        object->texture = dmnsn_new_texture();
      dmnsn_delete_pigment(object->texture->pigment);
      object->texture->pigment = dmnsn_realize_pigment(modifier);
      break;
    case DMNSN_AST_FINISH:
      if (!object->texture)
        object->texture = dmnsn_new_texture();
      dmnsn_delete_finish(object->texture->finish);
      object->texture->finish = dmnsn_realize_finish(modifier);
      break;

    case DMNSN_AST_INTERIOR:
      dmnsn_delete_interior(object->interior);
      object->interior = dmnsn_realize_interior(modifier);
      break;

    default:
      dmnsn_assert(false, "Invalid object modifier.");
    }
  }
}

static void
dmnsn_realize_light_source_modifiers(dmnsn_astnode astnode, dmnsn_light *light)
{
  dmnsn_assert(astnode.type == DMNSN_AST_OBJECT_MODIFIERS,
               "Expected object modifiers.");

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
    dmnsn_astnode modifier;
    dmnsn_array_get(astnode.children, i, &modifier);

    switch (modifier.type) {
    case DMNSN_AST_ROTATION:
      light->x0 = dmnsn_transform_vector(
        dmnsn_realize_rotation(modifier),
        light->x0
      );
      break;
    case DMNSN_AST_SCALE:
      light->x0 = dmnsn_transform_vector(
        dmnsn_realize_scale(modifier),
        light->x0
      );
      break;
    case DMNSN_AST_TRANSLATION:
      light->x0 = dmnsn_transform_vector(
        dmnsn_realize_translation(modifier),
        light->x0
      );
      break;

    case DMNSN_AST_TEXTURE:
    case DMNSN_AST_PIGMENT:
    case DMNSN_AST_FINISH:
    case DMNSN_AST_INTERIOR:
      dmnsn_diagnostic(modifier.location,
                       "WARNING: ignoring %s applied to light source",
                       dmnsn_astnode_string(modifier.type));
      break;

    default:
      dmnsn_assert(false, "Invalid object modifier.");
    }
  }
}

static dmnsn_light *
dmnsn_realize_light_source(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_LIGHT_SOURCE,
               "Expected a light source.");

  dmnsn_astnode point, color_node;
  dmnsn_array_get(astnode.children, 0, &point);
  dmnsn_array_get(astnode.children, 1, &color_node);

  dmnsn_vector x0 = dmnsn_realize_vector(point);
  dmnsn_color color = dmnsn_realize_color(color_node);

  dmnsn_light *light = dmnsn_new_point_light(x0, color);

  dmnsn_astnode modifiers;
  dmnsn_array_get(astnode.children, 2, &modifiers);
  dmnsn_realize_light_source_modifiers(modifiers, light);

  return light;
}

static dmnsn_object *dmnsn_realize_object(dmnsn_astnode astnode,
                                          dmnsn_array *lights);

static dmnsn_object *
dmnsn_realize_box(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_BOX, "Expected a box.");

  dmnsn_astnode corner1, corner2;
  dmnsn_array_get(astnode.children, 0, &corner1);
  dmnsn_array_get(astnode.children, 1, &corner2);

  dmnsn_vector x1 = dmnsn_realize_vector(corner1),
               x2 = dmnsn_realize_vector(corner2);

  dmnsn_object *box = dmnsn_new_cube();

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
  dmnsn_realize_object_modifiers(modifiers, box);

  return box;
}

static dmnsn_object *
dmnsn_realize_sphere(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_SPHERE, "Expected a sphere.");

  dmnsn_astnode center, radius;
  dmnsn_array_get(astnode.children, 0, &center);
  dmnsn_array_get(astnode.children, 1, &radius);

  dmnsn_vector x0 = dmnsn_realize_vector(center);
  double r = dmnsn_realize_float(radius);

  dmnsn_object *sphere = dmnsn_new_sphere();

  sphere->trans = dmnsn_scale_matrix(dmnsn_new_vector(r, r, r));
  sphere->trans = dmnsn_matrix_mul(dmnsn_translation_matrix(x0), sphere->trans);

  dmnsn_astnode modifiers;
  dmnsn_array_get(astnode.children, 2, &modifiers);
  dmnsn_realize_object_modifiers(modifiers, sphere);

  return sphere;
}

static dmnsn_object *
dmnsn_realize_plane(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_PLANE, "Expected a plane.");

  dmnsn_astnode normal, distance;
  dmnsn_array_get(astnode.children, 0, &normal);
  dmnsn_array_get(astnode.children, 1, &distance);

  dmnsn_vector n = dmnsn_vector_normalize(dmnsn_realize_vector(normal));
  double d = dmnsn_realize_float(distance);

  dmnsn_object *plane = dmnsn_new_plane(n);
  plane->trans = dmnsn_translation_matrix(dmnsn_vector_mul(d, n));

  dmnsn_astnode modifiers;
  dmnsn_array_get(astnode.children, 2, &modifiers);
  dmnsn_realize_object_modifiers(modifiers, plane);

  return plane;
}

typedef dmnsn_object *dmnsn_csg_object_fn(dmnsn_object *a, dmnsn_object *b);

/* Generalized CSG realizer */
static dmnsn_object *
dmnsn_realize_csg(dmnsn_astnode astnode, dmnsn_array *lights,
                  dmnsn_csg_object_fn *csg_object_fn)
{
  dmnsn_astnode objects, modifiers;
  dmnsn_array_get(astnode.children, 0, &objects);
  dmnsn_array_get(astnode.children, 1, &modifiers);

  unsigned int i;
  dmnsn_object *csg = NULL;
  for (i = 0; i < dmnsn_array_size(objects.children) && !csg; ++i) {
    dmnsn_astnode onode;
    dmnsn_array_get(objects.children, i, &onode);

    if (onode.type == DMNSN_AST_LIGHT_SOURCE) {
      dmnsn_light *light = dmnsn_realize_light_source(onode);
      dmnsn_realize_light_source_modifiers(modifiers, light);
      dmnsn_array_push(lights, &light);
    } else {
      csg = dmnsn_realize_object(onode, lights);
    }
  }

  for (; i < dmnsn_array_size(objects.children); ++i) {
    dmnsn_astnode onode;
    dmnsn_array_get(objects.children, i, &onode);

    if (onode.type == DMNSN_AST_LIGHT_SOURCE) {
      dmnsn_light *light = dmnsn_realize_light_source(onode);
      dmnsn_realize_light_source_modifiers(modifiers, light);
      dmnsn_array_push(lights, &light);
    } else {
      dmnsn_object *object = dmnsn_realize_object(onode, lights);
      csg = (*csg_object_fn)(csg, object);
    }
  }

  dmnsn_realize_object_modifiers(modifiers, csg);
  return csg;
}

static dmnsn_object *
dmnsn_realize_union(dmnsn_astnode astnode, dmnsn_array *lights)
{
  dmnsn_assert(astnode.type == DMNSN_AST_UNION, "Expected a union.");
  return dmnsn_realize_csg(astnode, lights, &dmnsn_new_csg_union);
}

static dmnsn_object *
dmnsn_realize_intersection(dmnsn_astnode astnode, dmnsn_array *lights)
{
  dmnsn_assert(astnode.type == DMNSN_AST_INTERSECTION,
               "Expected an intersection.");
  return dmnsn_realize_csg(astnode, lights, &dmnsn_new_csg_intersection);
}

static dmnsn_object *
dmnsn_realize_difference(dmnsn_astnode astnode, dmnsn_array *lights)
{
  dmnsn_assert(astnode.type == DMNSN_AST_DIFFERENCE, "Expected a difference.");
  return dmnsn_realize_csg(astnode, lights, &dmnsn_new_csg_difference);
}

static dmnsn_object *
dmnsn_realize_merge(dmnsn_astnode astnode, dmnsn_array *lights)
{
  dmnsn_assert(astnode.type == DMNSN_AST_MERGE, "Expected a merge.");
  return dmnsn_realize_csg(astnode, lights, &dmnsn_new_csg_merge);
}

/* Realize an object, or maybe a light */
static dmnsn_object *
dmnsn_realize_object(dmnsn_astnode astnode, dmnsn_array *lights)
{
  switch (astnode.type) {
  case DMNSN_AST_BOX:
    return dmnsn_realize_box(astnode);
  case DMNSN_AST_DIFFERENCE:
    return dmnsn_realize_difference(astnode, lights);
  case DMNSN_AST_INTERSECTION:
    return dmnsn_realize_intersection(astnode, lights);
  case DMNSN_AST_MERGE:
    return dmnsn_realize_merge(astnode, lights);
  case DMNSN_AST_PLANE:
    return dmnsn_realize_plane(astnode);
  case DMNSN_AST_SPHERE:
    return dmnsn_realize_sphere(astnode);
  case DMNSN_AST_UNION:
    return dmnsn_realize_union(astnode, lights);

  case DMNSN_AST_LIGHT_SOURCE:
    {
      dmnsn_light *light = dmnsn_realize_light_source(astnode);
      dmnsn_array_push(lights, &light);
      return NULL;
    }

  default:
    dmnsn_assert(false, "Expected an object.");
    return NULL; // Shut up compiler
  }
}

static dmnsn_scene *
dmnsn_realize_astree(const dmnsn_astree *astree)
{
  dmnsn_scene *scene = dmnsn_new_scene();

  /* Default finish */
  scene->default_texture->finish = dmnsn_new_finish_combination(
    dmnsn_new_ambient_finish(
      dmnsn_color_mul(0.1, dmnsn_white)
    ),
    dmnsn_new_diffuse_finish(0.6)
  );

  /* Background color */
  scene->background = dmnsn_black;

  /* Create the default perspective camera */
  scene->camera = dmnsn_new_perspective_camera();

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
    case DMNSN_AST_GLOBAL_SETTINGS:
      dmnsn_realize_global_settings(astnode, scene);
      break;

    case DMNSN_AST_BACKGROUND:
      dmnsn_array_get(astnode.children, 0, &astnode);
      scene->background = dmnsn_realize_color(astnode);
      break;

    case DMNSN_AST_CAMERA:
      dmnsn_delete_camera(scene->camera);
      scene->camera = dmnsn_realize_camera(astnode);
      break;

    case DMNSN_AST_BOX:
    case DMNSN_AST_DIFFERENCE:
    case DMNSN_AST_INTERSECTION:
    case DMNSN_AST_MERGE:
    case DMNSN_AST_PLANE:
    case DMNSN_AST_SPHERE:
    case DMNSN_AST_UNION:
      object = dmnsn_realize_object(astnode, scene->lights);
      if (object)
        dmnsn_array_push(scene->objects, &object);
      break;

    case DMNSN_AST_LIGHT_SOURCE:
      light = dmnsn_realize_light_source(astnode);
      dmnsn_array_push(scene->lights, &light);
      break;

    default:
      dmnsn_assert(false, "Unrecognised syntax element.");
    }
  }

  return scene;
}

dmnsn_scene *
dmnsn_realize(FILE *file, dmnsn_symbol_table *symtable)
{
  if (!symtable) {
    symtable = dmnsn_new_symbol_table();
  }

  dmnsn_astree *astree = dmnsn_parse(file, symtable);
  if (!astree) {
    return NULL;
  }

  dmnsn_scene *scene = dmnsn_realize_astree(astree);

  dmnsn_delete_astree(astree);
  return scene;
}

dmnsn_scene *
dmnsn_realize_string(const char *str, dmnsn_symbol_table *symtable)
{
  if (!symtable) {
    symtable = dmnsn_new_symbol_table();
  }

  dmnsn_astree *astree = dmnsn_parse_string(str, symtable);
  if (!astree) {
    return NULL;
  }

  dmnsn_scene *scene = dmnsn_realize_astree(astree);

  dmnsn_delete_astree(astree);
  return scene;
}
