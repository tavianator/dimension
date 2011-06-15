/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Finishes.
 */

#include "dimension.h"

dmnsn_ambient *
dmnsn_new_ambient(void)
{
  dmnsn_ambient *ambient = dmnsn_malloc(sizeof(dmnsn_ambient));
  ambient->free_fn  = NULL;
  ambient->ptr      = NULL;
  ambient->refcount = 1;
  return ambient;
}

void
dmnsn_delete_ambient(dmnsn_ambient *ambient)
{
  if (DMNSN_DECREF(ambient)) {
    if (ambient->free_fn) {
      ambient->free_fn(ambient->ptr);
    }
    dmnsn_free(ambient);
  }
}

dmnsn_diffuse *
dmnsn_new_diffuse(void)
{
  dmnsn_diffuse *diffuse = dmnsn_malloc(sizeof(dmnsn_diffuse));
  diffuse->free_fn  = NULL;
  diffuse->ptr      = NULL;
  diffuse->refcount = 1;
  return diffuse;
}

void
dmnsn_delete_diffuse(dmnsn_diffuse *diffuse)
{
  if (DMNSN_DECREF(diffuse)) {
    if (diffuse->free_fn) {
      diffuse->free_fn(diffuse->ptr);
    }
    dmnsn_free(diffuse);
  }
}

dmnsn_specular *
dmnsn_new_specular(void)
{
  dmnsn_specular *specular = dmnsn_malloc(sizeof(dmnsn_specular));
  specular->free_fn  = NULL;
  specular->ptr      = NULL;
  specular->refcount = 1;
  return specular;
}

void
dmnsn_delete_specular(dmnsn_specular *specular)
{
  if (DMNSN_DECREF(specular)) {
    if (specular->free_fn) {
      specular->free_fn(specular->ptr);
    }
    dmnsn_free(specular);
  }
}

dmnsn_reflection *
dmnsn_new_reflection(void)
{
  dmnsn_reflection *reflection = dmnsn_malloc(sizeof(dmnsn_reflection));
  reflection->free_fn  = NULL;
  reflection->ptr      = NULL;
  reflection->refcount = 1;
  return reflection;
}

void
dmnsn_delete_reflection(dmnsn_reflection *reflection)
{
  if (DMNSN_DECREF(reflection)) {
    if (reflection->free_fn) {
      reflection->free_fn(reflection->ptr);
    }
    dmnsn_free(reflection);
  }
}

dmnsn_finish
dmnsn_new_finish(void)
{
  dmnsn_finish finish;
  finish.ambient    = NULL;
  finish.diffuse    = NULL;
  finish.specular   = NULL;
  finish.reflection = NULL;
  return finish;
}

void
dmnsn_delete_finish(dmnsn_finish finish)
{
  dmnsn_delete_reflection(finish.reflection);
  dmnsn_delete_specular(finish.specular);
  dmnsn_delete_diffuse(finish.diffuse);
  dmnsn_delete_ambient(finish.ambient);
}

void
dmnsn_finish_incref(dmnsn_finish *finish)
{
  if (finish->ambient) {
    DMNSN_INCREF(finish->ambient);
  }
  if (finish->diffuse) {
    DMNSN_INCREF(finish->diffuse);
  }
  if (finish->specular) {
    DMNSN_INCREF(finish->specular);
  }
  if (finish->reflection) {
    DMNSN_INCREF(finish->reflection);
  }

}

void
dmnsn_finish_cascade(const dmnsn_finish *default_finish, dmnsn_finish *finish)
{
  if (!finish->ambient) {
    finish->ambient = default_finish->ambient;
    DMNSN_INCREF(finish->ambient);
  }
  if (!finish->diffuse) {
    finish->diffuse = default_finish->diffuse;
    DMNSN_INCREF(finish->diffuse);
  }
  if (!finish->specular) {
    finish->specular = default_finish->specular;
    DMNSN_INCREF(finish->specular);
  }
  if (!finish->reflection) {
    finish->reflection = default_finish->reflection;
    DMNSN_INCREF(finish->reflection);
  }
}
