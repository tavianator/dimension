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
#include <math.h>

/*
 * Finish combinations
 */

static dmnsn_color
dmnsn_finish_combination_fn(const dmnsn_finish *finish,
                            dmnsn_color light, dmnsn_color color,
                            dmnsn_vector ray, dmnsn_vector normal,
                            dmnsn_vector viewer)
{
  dmnsn_finish **params = finish->ptr;
  if (params[0]->finish_fn && params[1]->finish_fn) {
    return dmnsn_color_add((*params[0]->finish_fn)(params[0], light, color, ray,
                                                   normal, viewer),
                           (*params[1]->finish_fn)(params[1], light, color, ray,
                                                   normal, viewer));
  } else if (params[0]->finish_fn) {
    return (*params[0]->finish_fn)(params[0], light, color, ray,
                                   normal, viewer);
  } else if (params[1]->finish_fn) {
    return (*params[1]->finish_fn)(params[1], light, color, ray,
                                   normal, viewer);
  } else {
    return dmnsn_black;
  }
}

static dmnsn_color
dmnsn_finish_combination_ambient_fn(const dmnsn_finish *finish,
                                    dmnsn_color pigment)
{
  dmnsn_finish **params = finish->ptr;
  if (params[0]->ambient_fn && params[1]->ambient_fn) {
    return dmnsn_color_add((*params[0]->ambient_fn)(params[0], pigment),
                           (*params[1]->ambient_fn)(params[1], pigment));
  } else if (params[0]->ambient_fn) {
    return (*params[0]->ambient_fn)(params[0], pigment);
  } else if (params[1]->ambient_fn) {
    return (*params[1]->ambient_fn)(params[1], pigment);
  } else {
    return dmnsn_black;
  }
}

static void
dmnsn_finish_combination_free_fn(void *ptr)
{
  dmnsn_finish **params = ptr;
  dmnsn_delete_finish(params[0]);
  dmnsn_delete_finish(params[1]);
  free(ptr);
}

dmnsn_finish *
dmnsn_new_finish_combination(dmnsn_finish *f1, dmnsn_finish *f2)
{
  dmnsn_finish *finish = dmnsn_new_finish();
  if (finish) {
    dmnsn_finish **params = malloc(2*sizeof(dmnsn_finish *));
    if (!params) {
      dmnsn_delete_finish(finish);
      return NULL;
    }

    params[0] = f1;
    params[1] = f2;

    finish->ptr        = params;
    finish->finish_fn  = &dmnsn_finish_combination_fn;
    finish->ambient_fn = &dmnsn_finish_combination_ambient_fn;
    finish->free_fn    = &dmnsn_finish_combination_free_fn;
  }
  return finish;
}

/*
 * Ambient finish
 */

static dmnsn_color
dmnsn_ambient_finish_fn(const dmnsn_finish *finish, dmnsn_color pigment)
{
  dmnsn_color *ambient = finish->ptr;
  return dmnsn_color_illuminate(*ambient, pigment);
}

dmnsn_finish *
dmnsn_new_ambient_finish(dmnsn_color ambient)
{
  dmnsn_finish *finish = dmnsn_new_finish();
  if (finish) {
    dmnsn_color *param = malloc(sizeof(dmnsn_color));
    if (!param) {
      dmnsn_delete_finish(finish);
      return NULL;
    }

    *param = ambient;
    finish->ptr        = param;
    finish->ambient_fn = &dmnsn_ambient_finish_fn;
    finish->free_fn    = &free;
  }
  return finish;
}

/*
 * Diffuse finish
 */

static dmnsn_color
dmnsn_diffuse_finish_fn(const dmnsn_finish *finish,
                        dmnsn_color light, dmnsn_color color,
                        dmnsn_vector ray, dmnsn_vector normal,
                        dmnsn_vector viewer)
{
  double *diffuse = finish->ptr;
  double diffuse_factor = (*diffuse)*dmnsn_vector_dot(ray, normal);
  return dmnsn_color_mul(diffuse_factor, dmnsn_color_illuminate(light, color));
}

dmnsn_finish *
dmnsn_new_diffuse_finish(double diffuse)
{
  dmnsn_finish *finish = dmnsn_new_finish();
  if (finish) {
    double *param = malloc(sizeof(double));
    if (!param) {
      dmnsn_delete_finish(finish);
      return NULL;
    }

    *param = diffuse;

    finish->ptr       = param;
    finish->finish_fn = &dmnsn_diffuse_finish_fn;
    finish->free_fn   = &free;
  }
  return finish;
}

/*
 * Phong finish
 */

static dmnsn_color
dmnsn_phong_finish_fn(const dmnsn_finish *finish,
                      dmnsn_color light, dmnsn_color color,
                      dmnsn_vector ray, dmnsn_vector normal,
                      dmnsn_vector viewer)
{
  double *params = finish->ptr;

  double specular = params[0];
  double exp      = params[1];

  dmnsn_vector proj = dmnsn_vector_mul(2*dmnsn_vector_dot(ray, normal), normal);
  dmnsn_vector reflected = dmnsn_vector_sub(proj, ray);

  double specular_factor = pow(dmnsn_vector_dot(reflected, viewer), exp);
  return dmnsn_color_mul(specular*specular_factor, light);
}

/* A phong finish */
dmnsn_finish *
dmnsn_new_phong_finish(double specular, double exp)
{
  dmnsn_finish *finish = dmnsn_new_finish();
  if (finish) {
    double *params = malloc(2*sizeof(double));
    if (!params) {
      dmnsn_delete_finish(finish);
      return NULL;
    }

    params[0] = specular;
    params[1] = exp;

    finish->ptr       = params;
    finish->finish_fn = &dmnsn_phong_finish_fn;
    finish->free_fn   = &free;
  }
  return finish;
}
