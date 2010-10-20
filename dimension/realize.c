/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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
#include <fenv.h>
#include <stdio.h>
#include <stdbool.h>

static long
dmnsn_realize_integer(dmnsn_astnode astnode)
{
  switch (astnode.type) {
  case DMNSN_AST_INTEGER:
    return *(long *)astnode.ptr;
  case DMNSN_AST_FLOAT:
    {
      feclearexcept(FE_ALL_EXCEPT);
      long ret = lrint(*(double *)astnode.ptr);
      if (fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW))
        dmnsn_error(DMNSN_SEVERITY_HIGH, "Float out of range of integer.");
      return ret;
    }

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
dmnsn_realize_scale(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_SCALE, "Expected a scale.");

  dmnsn_astnode scale_node;
  dmnsn_array_get(astnode.children, 0, &scale_node);
  dmnsn_vector scale = dmnsn_realize_vector(scale_node);

  return dmnsn_scale_matrix(scale);
}

static dmnsn_matrix
dmnsn_realize_rotation(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_ROTATION, "Expected a rotation.");

  dmnsn_astnode angle_node;
  dmnsn_array_get(astnode.children, 0, &angle_node);

  dmnsn_vector angle = dmnsn_vector_mul(
    dmnsn_radians(1.0),
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
dmnsn_realize_matrix(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_MATRIX, "Expected a matrix.");

  dmnsn_astnode *children = dmnsn_array_first(astnode.children);
  dmnsn_matrix trans;

  trans.n[0][0] = dmnsn_realize_float(children[0]);
  trans.n[0][1] = dmnsn_realize_float(children[1]);
  trans.n[0][2] = dmnsn_realize_float(children[2]);
  trans.n[0][3] = dmnsn_realize_float(children[9]);

  trans.n[1][0] = dmnsn_realize_float(children[3]);
  trans.n[1][1] = dmnsn_realize_float(children[4]);
  trans.n[1][2] = dmnsn_realize_float(children[5]);
  trans.n[1][3] = dmnsn_realize_float(children[10]);

  trans.n[2][0] = dmnsn_realize_float(children[6]);
  trans.n[2][1] = dmnsn_realize_float(children[7]);
  trans.n[2][2] = dmnsn_realize_float(children[8]);
  trans.n[2][3] = dmnsn_realize_float(children[11]);

  trans.n[3][0] = 0.0;
  trans.n[3][1] = 0.0;
  trans.n[3][2] = 0.0;
  trans.n[3][3] = 1.0;

  return trans;
}

static dmnsn_matrix
dmnsn_realize_transformation(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_TRANSFORMATION,
               "Expected a transformation.");

  dmnsn_matrix trans = dmnsn_identity_matrix();

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, child, astnode.children) {
    switch (child->type) {
    case DMNSN_AST_TRANSLATION:
      trans = dmnsn_matrix_mul(trans, dmnsn_realize_translation(*child));
      break;
    case DMNSN_AST_SCALE:
      trans = dmnsn_matrix_mul(trans, dmnsn_realize_scale(*child));
      break;
    case DMNSN_AST_ROTATION:
      trans = dmnsn_matrix_mul(trans, dmnsn_realize_rotation(*child));
      break;
    case DMNSN_AST_MATRIX:
      trans = dmnsn_matrix_mul(trans, dmnsn_realize_matrix(*child));
      break;
    case DMNSN_AST_INVERSE:
      trans = dmnsn_matrix_inverse(trans);
      break;
    case DMNSN_AST_TRANSFORMATION:
      trans = dmnsn_matrix_mul(trans, dmnsn_realize_transformation(*child));
      break;

    default:
      dmnsn_assert(false, "Invalid transformation type.");
      break;
    }
  }

  return trans;
}

static void
dmnsn_realize_global_settings(dmnsn_astnode astnode, dmnsn_scene *scene)
{
  dmnsn_assert(astnode.type == DMNSN_AST_GLOBAL_SETTINGS,
               "Expected global settings.");

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, item, astnode.children) {
    dmnsn_astnode child;

    switch (item->type) {
    case DMNSN_AST_ASSUMED_GAMMA:
      /* assumed_gamma not supported */
      break;

    case DMNSN_AST_MAX_TRACE_LEVEL:
      dmnsn_array_get(item->children, 0, &child);
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

  dmnsn_astnode_type camera_type = DMNSN_AST_PERSPECTIVE;
  dmnsn_vector location  = dmnsn_new_vector(0.0, 0.0, 0.0);
  dmnsn_vector direction = dmnsn_new_vector(0.0, 0.0, 1.0);
  dmnsn_vector right     = dmnsn_new_vector(4.0/3.0, 0.0, 0.0);
  dmnsn_vector up        = dmnsn_new_vector(0.0, 1.0, 0.0);
  dmnsn_vector sky       = dmnsn_new_vector(0.0, 1.0, 0.0);
  dmnsn_matrix trans     = dmnsn_identity_matrix();

  dmnsn_camera *camera = NULL;

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, item, astnode.children) {
    dmnsn_astnode child;

    switch (item->type) {
    /* Camera types */
    case DMNSN_AST_PERSPECTIVE:
      camera_type = item->type;
      break;

    /* Camera vectors */
    case DMNSN_AST_LOCATION:
      dmnsn_array_get(item->children, 0, &child);
      location = dmnsn_realize_vector(child);
      break;
    case DMNSN_AST_RIGHT:
      dmnsn_array_get(item->children, 0, &child);
      right = dmnsn_realize_vector(child);
      break;
    case DMNSN_AST_UP:
      dmnsn_array_get(item->children, 0, &child);
      right = dmnsn_realize_vector(child);
      break;
    case DMNSN_AST_SKY:
      dmnsn_array_get(item->children, 0, &child);
      sky = dmnsn_realize_vector(child);
      break;
    case DMNSN_AST_DIRECTION:
      dmnsn_array_get(item->children, 0, &child);
      direction = dmnsn_realize_vector(child);
      break;

    /* Camera modifiers */

    case DMNSN_AST_LOOK_AT:
      {
        dmnsn_array_get(item->children, 0, &child);
        dmnsn_vector look_at = dmnsn_realize_vector(child);

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
        dmnsn_array_get(item->children, 0, &child);
        double angle = dmnsn_radians(dmnsn_realize_float(child));
        direction = dmnsn_vector_mul(
          0.5*dmnsn_vector_norm(right)/tan(angle/2.0),
          dmnsn_vector_normalize(direction)
        );
        break;
      }

    /* Transformations */
    case DMNSN_AST_TRANSFORMATION:
      trans = dmnsn_matrix_mul(dmnsn_realize_transformation(*item), trans);
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

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, modifier, astnode.children) {
    switch (modifier->type) {
    case DMNSN_AST_TRANSFORMATION:
      pigment->trans = dmnsn_matrix_mul(
        dmnsn_realize_transformation(*modifier),
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

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, item, items.children) {
    dmnsn_astnode child;

    switch (item->type) {
    case DMNSN_AST_FALLOFF:
      dmnsn_array_get(item->children, 0, &child);
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

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, item, astnode.children) {
    dmnsn_astnode child;

    switch (item->type) {
    case DMNSN_AST_AMBIENT:
      dmnsn_array_get(item->children, 0, &child);
      ambient = dmnsn_realize_color(child);
      ambient_set = true;
      break;

    case DMNSN_AST_DIFFUSE:
      dmnsn_array_get(item->children, 0, &child);
      diffuse = dmnsn_realize_float(child);
      diffuse_set = true;
      break;

    case DMNSN_AST_PHONG:
      dmnsn_array_get(item->children, 0, &child);
      phong = dmnsn_realize_float(child);
      break;
    case DMNSN_AST_PHONG_SIZE:
      dmnsn_array_get(item->children, 0, &child);
      phong_size = dmnsn_realize_float(child);
      break;

    case DMNSN_AST_REFLECTION:
      dmnsn_delete_finish(reflection);
      reflection = dmnsn_realize_reflection(*item);
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

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, item, astnode.children) {
    switch (item->type) {
    case DMNSN_AST_PIGMENT:
      dmnsn_delete_pigment(texture->pigment);
      texture->pigment = dmnsn_realize_pigment(*item);
      break;

    case DMNSN_AST_FINISH:
      dmnsn_delete_finish(texture->finish);
      texture->finish = dmnsn_realize_finish(*item);
      break;

    case DMNSN_AST_TRANSFORMATION:
      texture->trans = dmnsn_matrix_mul(
        dmnsn_realize_transformation(*item),
        texture->trans
      );
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

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, item, astnode.children) {
    dmnsn_astnode child;

    switch (item->type) {
    case DMNSN_AST_IOR:
      dmnsn_array_get(item->children, 0, &child);
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

  /* Save the pre-existing transformations */
  dmnsn_matrix existing_trans = dmnsn_matrix_inverse(object->trans);

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, modifier, astnode.children) {
    switch (modifier->type) {
    case DMNSN_AST_TRANSFORMATION:
      object->trans = dmnsn_matrix_mul(
        dmnsn_realize_transformation(*modifier),
        object->trans
      );
      break;

    case DMNSN_AST_TEXTURE:
      dmnsn_delete_texture(object->texture);
      object->texture = dmnsn_realize_texture(*modifier);
      break;
    case DMNSN_AST_PIGMENT:
      if (!object->texture)
        object->texture = dmnsn_new_texture();
      dmnsn_delete_pigment(object->texture->pigment);
      object->texture->pigment = dmnsn_realize_pigment(*modifier);
      break;
    case DMNSN_AST_FINISH:
      if (!object->texture)
        object->texture = dmnsn_new_texture();
      dmnsn_delete_finish(object->texture->finish);
      object->texture->finish = dmnsn_realize_finish(*modifier);
      break;

    case DMNSN_AST_INTERIOR:
      dmnsn_delete_interior(object->interior);
      object->interior = dmnsn_realize_interior(*modifier);
      break;

    default:
      dmnsn_assert(false, "Invalid object modifier.");
    }
  }

  if (object->texture) {
    /* Right-multiply to counteract any pre-existing transformations -- this
       means, for example, that the transformation that makes a sphere have
       radius 2 doesn't scale the texture by a factor of 2 */
    object->texture->trans = dmnsn_matrix_mul(
      object->texture->trans,
      existing_trans
    );
  }
}

static void
dmnsn_realize_light_source_modifiers(dmnsn_astnode astnode, dmnsn_light *light)
{
  dmnsn_assert(astnode.type == DMNSN_AST_OBJECT_MODIFIERS,
               "Expected object modifiers.");

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, modifier, astnode.children) {
    switch (modifier->type) {
    case DMNSN_AST_TRANSFORMATION:
      light->x0 = dmnsn_transform_vector(
        dmnsn_realize_transformation(*modifier),
        light->x0
      );
      break;

    case DMNSN_AST_TEXTURE:
    case DMNSN_AST_PIGMENT:
    case DMNSN_AST_FINISH:
    case DMNSN_AST_INTERIOR:
      /* Ignore other object modifiers */
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

  return box;
}

static dmnsn_object *
dmnsn_realize_cylinder(dmnsn_astnode astnode)
{
  dmnsn_assert(astnode.type == DMNSN_AST_CYLINDER, "Expected a cylinder.");

  dmnsn_astnode pnode1, pnode2, radius, open;
  dmnsn_array_get(astnode.children, 0, &pnode1);
  dmnsn_array_get(astnode.children, 1, &pnode2);
  dmnsn_array_get(astnode.children, 2, &radius);
  dmnsn_array_get(astnode.children, 3, &open);

  dmnsn_vector p1 = dmnsn_realize_vector(pnode1);
  dmnsn_vector p2 = dmnsn_realize_vector(pnode2);
  double r = dmnsn_realize_float(radius);

  dmnsn_vector dir = dmnsn_vector_sub(p2, p1);
  double l = dmnsn_vector_norm(dir);

  double theta1 = dmnsn_vector_axis_angle(dmnsn_y, dir, dmnsn_x);
  double theta2 = dmnsn_vector_axis_angle(dmnsn_y, dir, dmnsn_z);

  dmnsn_object *cylinder
    = dmnsn_new_cylinder(r, r, dmnsn_realize_integer(open));
  /* Transformations: lift the cylinder to start at the origin, scale, rotate,
     and translate properly */
  cylinder->trans = dmnsn_translation_matrix(dmnsn_new_vector(0.0, 1.0, 0.0));
  cylinder->trans = dmnsn_matrix_mul(
    dmnsn_scale_matrix(dmnsn_new_vector(1.0, l/2.0, 1.0)),
    cylinder->trans
  );
  cylinder->trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(theta1, 0.0, 0.0)),
    cylinder->trans
  );
  cylinder->trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(0.0, 0.0, theta2)),
    cylinder->trans
  );
  cylinder->trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(p1),
    cylinder->trans
  );
  return cylinder;
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
  return plane;
}

/* Bulk-load a union */
static dmnsn_object *
dmnsn_realize_union(dmnsn_astnode astnode, dmnsn_astnode modifiers,
                    dmnsn_array *lights)
{
  dmnsn_assert(astnode.type == DMNSN_AST_UNION, "Expected a union.");

  dmnsn_array *children = dmnsn_new_array(sizeof(dmnsn_object *));
  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, onode, astnode.children) {
    if (onode->type == DMNSN_AST_LIGHT_SOURCE) {
      dmnsn_light *light = dmnsn_realize_light_source(*onode);
      dmnsn_realize_light_source_modifiers(modifiers, light);
      dmnsn_array_push(lights, &light);
    } else {
      dmnsn_object *object = dmnsn_realize_object(*onode, lights);
      if (object)
        dmnsn_array_push(children, &object);
    }
  }

  dmnsn_object *csg = NULL;
  if (dmnsn_array_size(children) > 0)
    csg = dmnsn_new_csg_union(children);
  dmnsn_delete_array(children);
  return csg;
}

typedef dmnsn_object *dmnsn_csg_object_fn(dmnsn_object *a, dmnsn_object *b);

/* Generalized CSG realizer */
static dmnsn_object *
dmnsn_realize_csg(dmnsn_astnode astnode, dmnsn_astnode modifiers,
                  dmnsn_array *lights, dmnsn_csg_object_fn *csg_object_fn)
{
  dmnsn_object *csg = NULL;
  dmnsn_astnode *onode;
  for (onode = dmnsn_array_first(astnode.children);
       onode <= (dmnsn_astnode *)dmnsn_array_last(astnode.children);
       ++onode)
  {
    if (onode->type == DMNSN_AST_LIGHT_SOURCE) {
      dmnsn_light *light = dmnsn_realize_light_source(*onode);
      dmnsn_realize_light_source_modifiers(modifiers, light);
      dmnsn_array_push(lights, &light);
    } else {
      csg = dmnsn_realize_object(*onode, lights);
      break;
    }
  }

  for (++onode;
       onode <= (dmnsn_astnode *)dmnsn_array_last(astnode.children);
       ++onode)
  {
    if (onode->type == DMNSN_AST_LIGHT_SOURCE) {
      dmnsn_light *light = dmnsn_realize_light_source(*onode);
      dmnsn_realize_light_source_modifiers(modifiers, light);
      dmnsn_array_push(lights, &light);
    } else {
      dmnsn_object *object = dmnsn_realize_object(*onode, lights);
      csg = (*csg_object_fn)(csg, object);
    }
  }

  return csg;
}

static dmnsn_object *
dmnsn_realize_intersection(dmnsn_astnode astnode, dmnsn_astnode modifiers,
                           dmnsn_array *lights)
{
  dmnsn_assert(astnode.type == DMNSN_AST_INTERSECTION,
               "Expected an intersection.");
  return dmnsn_realize_csg(astnode, modifiers, lights,
                           &dmnsn_new_csg_intersection);
}

static dmnsn_object *
dmnsn_realize_difference(dmnsn_astnode astnode, dmnsn_astnode modifiers,
                         dmnsn_array *lights)
{
  dmnsn_assert(astnode.type == DMNSN_AST_DIFFERENCE, "Expected a difference.");
  return dmnsn_realize_csg(astnode, modifiers, lights,
                           &dmnsn_new_csg_difference);
}

static dmnsn_object *
dmnsn_realize_merge(dmnsn_astnode astnode, dmnsn_astnode modifiers,
                    dmnsn_array *lights)
{
  dmnsn_assert(astnode.type == DMNSN_AST_MERGE, "Expected a merge.");
  return dmnsn_realize_csg(astnode, modifiers, lights, &dmnsn_new_csg_merge);
}

/* Realize an object, or maybe a light */
static dmnsn_object *
dmnsn_realize_object(dmnsn_astnode astnode, dmnsn_array *lights)
{
  dmnsn_assert(astnode.type == DMNSN_AST_OBJECT
               || astnode.type == DMNSN_AST_LIGHT_SOURCE,
               "Expected an object.");

  dmnsn_astnode onode, modifiers;
  dmnsn_array_get(astnode.children, 0, &onode);
  dmnsn_array_get(astnode.children, 1, &modifiers);

  dmnsn_object *object = NULL;

  switch (onode.type) {
  case DMNSN_AST_BOX:
    object = dmnsn_realize_box(onode);
    break;
  case DMNSN_AST_CYLINDER:
    object = dmnsn_realize_cylinder(onode);
    break;
  case DMNSN_AST_DIFFERENCE:
    object = dmnsn_realize_difference(onode, modifiers, lights);
    break;
  case DMNSN_AST_INTERSECTION:
    object = dmnsn_realize_intersection(onode, modifiers, lights);
    break;
  case DMNSN_AST_MERGE:
    object = dmnsn_realize_merge(onode, modifiers, lights);
    break;
  case DMNSN_AST_PLANE:
    object = dmnsn_realize_plane(onode);
    break;
  case DMNSN_AST_SPHERE:
    object = dmnsn_realize_sphere(onode);
    break;
  case DMNSN_AST_UNION:
    object = dmnsn_realize_union(onode, modifiers, lights);
    break;

  case DMNSN_AST_LIGHT_SOURCE:
    {
      dmnsn_light *light = dmnsn_realize_light_source(astnode);
      dmnsn_array_push(lights, &light);
      return NULL;
    }

  default:
    dmnsn_assert(false, "Expected an object type.");
  }

  if (object) {
    dmnsn_realize_object_modifiers(modifiers, object);
  }
  return object;
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

  DMNSN_ARRAY_FOREACH (dmnsn_astnode *, astnode, astree) {
    dmnsn_astnode child;
    dmnsn_light *light;
    dmnsn_object *object;

    switch (astnode->type) {
    case DMNSN_AST_GLOBAL_SETTINGS:
      dmnsn_realize_global_settings(*astnode, scene);
      break;

    case DMNSN_AST_BACKGROUND:
      dmnsn_array_get(astnode->children, 0, &child);
      scene->background = dmnsn_realize_color(child);
      break;

    case DMNSN_AST_CAMERA:
      dmnsn_delete_camera(scene->camera);
      scene->camera = dmnsn_realize_camera(*astnode);
      break;

    case DMNSN_AST_OBJECT:
      object = dmnsn_realize_object(*astnode, scene->lights);
      if (object)
        dmnsn_array_push(scene->objects, &object);
      break;

    case DMNSN_AST_LIGHT_SOURCE:
      light = dmnsn_realize_light_source(*astnode);
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
