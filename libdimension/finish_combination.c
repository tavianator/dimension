/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@tavianator.com>          *
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

/**
 * @file
 * Finish combinations.
 */

#include "dimension.h"
#include <math.h>
#include <stdlib.h>

/** Diffuse combination callback. */
static dmnsn_color
dmnsn_finish_combination_diffuse_fn(const dmnsn_finish *finish,
                                    dmnsn_color light, dmnsn_color color,
                                    dmnsn_vector ray, dmnsn_vector normal)
{
  dmnsn_finish **params = finish->ptr;
  if (params[0]->diffuse_fn && params[1]->diffuse_fn) {
    return dmnsn_color_add(
      (*params[0]->diffuse_fn)(params[0], light, color, ray, normal),
      (*params[1]->diffuse_fn)(params[1], light, color, ray, normal)
    );
  } else if (params[0]->diffuse_fn) {
    return (*params[0]->diffuse_fn)(params[0], light, color, ray, normal);
  } else if (params[1]->diffuse_fn) {
    return (*params[1]->diffuse_fn)(params[1], light, color, ray, normal);
  } else {
    return dmnsn_black;
  }
}

/** Specular combination callback. */
static dmnsn_color
dmnsn_finish_combination_specular_fn(const dmnsn_finish *finish,
                                     dmnsn_color light, dmnsn_color color,
                                     dmnsn_vector ray, dmnsn_vector normal,
                                     dmnsn_vector viewer)
{
  dmnsn_finish **params = finish->ptr;
  if (params[0]->specular_fn && params[1]->specular_fn) {
    return dmnsn_color_add(
      (*params[0]->specular_fn)(params[0], light, color, ray, normal, viewer),
      (*params[1]->specular_fn)(params[1], light, color, ray, normal, viewer)
    );
  } else if (params[0]->specular_fn) {
    return (*params[0]->specular_fn)(params[0], light, color, ray,
                                     normal, viewer);
  } else if (params[1]->specular_fn) {
    return (*params[1]->specular_fn)(params[1], light, color, ray,
                                     normal, viewer);
  } else {
    return dmnsn_black;
  }
}

/** Ambient combination callback. */
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

/** Reflection combination callback. */
static dmnsn_color
dmnsn_finish_combination_reflection_fn(const dmnsn_finish *finish,
                                       dmnsn_color reflect, dmnsn_color color,
                                       dmnsn_vector ray, dmnsn_vector normal)
{
  dmnsn_finish **params = finish->ptr;
  if (params[0]->reflection_fn && params[1]->reflection_fn) {
    return dmnsn_color_add(
      (*params[0]->reflection_fn)(params[0], reflect, color, ray, normal),
      (*params[1]->reflection_fn)(params[1], reflect, color, ray, normal)
    );
  } else if (params[0]->reflection_fn) {
    return (*params[0]->reflection_fn)(params[0], reflect, color, ray, normal);
  } else if (params[1]->reflection_fn) {
    return (*params[1]->reflection_fn)(params[1], reflect, color, ray, normal);
  } else {
    return dmnsn_black;
  }
}

/** Finish combination destructor callback. */
static void
dmnsn_finish_combination_free_fn(void *ptr)
{
  dmnsn_finish **params = ptr;
  dmnsn_delete_finish(params[0]);
  dmnsn_delete_finish(params[1]);
  dmnsn_free(ptr);
}

dmnsn_finish *
dmnsn_new_finish_combination(dmnsn_finish *f1, dmnsn_finish *f2)
{
  dmnsn_finish *finish = dmnsn_new_finish();

  dmnsn_finish **params = dmnsn_malloc(2*sizeof(dmnsn_finish *));
  params[0] = f1;
  params[1] = f2;

  finish->ptr = params;

  if (f1->diffuse_fn || f2->diffuse_fn)
    finish->diffuse_fn = &dmnsn_finish_combination_diffuse_fn;

  if (f1->specular_fn || f2->specular_fn)
    finish->specular_fn = &dmnsn_finish_combination_specular_fn;

  if (f1->ambient_fn || f2->ambient_fn)
    finish->ambient_fn = &dmnsn_finish_combination_ambient_fn;

  if (f1->reflection_fn || f2->reflection_fn)
    finish->reflection_fn = &dmnsn_finish_combination_reflection_fn;

  finish->free_fn = &dmnsn_finish_combination_free_fn;

  return finish;
}
