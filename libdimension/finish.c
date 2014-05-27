/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "dimension-internal.h"

dmnsn_ambient *
dmnsn_new_ambient(dmnsn_color ambient_light)
{
  dmnsn_ambient *ambient = DMNSN_MALLOC(dmnsn_ambient);
  ambient->ambient = ambient_light;
  DMNSN_REFCOUNT_INIT(ambient);
  return ambient;
}

void
dmnsn_delete_ambient(dmnsn_ambient *ambient)
{
  if (DMNSN_DECREF(ambient)) {
    dmnsn_free(ambient);
  }
}

static void
dmnsn_default_diffuse_free_fn(dmnsn_diffuse *diffuse)
{
  dmnsn_free(diffuse);
}

dmnsn_diffuse *
dmnsn_new_diffuse(void)
{
  dmnsn_diffuse *diffuse = DMNSN_MALLOC(dmnsn_diffuse);
  dmnsn_init_diffuse(diffuse);
  return diffuse;
}

void
dmnsn_init_diffuse(dmnsn_diffuse *diffuse)
{
  diffuse->free_fn = dmnsn_default_diffuse_free_fn;
  DMNSN_REFCOUNT_INIT(diffuse);
}

void
dmnsn_delete_diffuse(dmnsn_diffuse *diffuse)
{
  if (DMNSN_DECREF(diffuse)) {
    diffuse->free_fn(diffuse);
  }
}

static void
dmnsn_default_specular_free_fn(dmnsn_specular *specular)
{
  dmnsn_free(specular);
}

dmnsn_specular *
dmnsn_new_specular(void)
{
  dmnsn_specular *specular = DMNSN_MALLOC(dmnsn_specular);
  dmnsn_init_specular(specular);
  return specular;
}

void
dmnsn_init_specular(dmnsn_specular *specular)
{
  specular->free_fn = dmnsn_default_specular_free_fn;
  DMNSN_REFCOUNT_INIT(specular);
}

void
dmnsn_delete_specular(dmnsn_specular *specular)
{
  if (DMNSN_DECREF(specular)) {
    specular->free_fn(specular);
  }
}

static void
dmnsn_default_reflection_free_fn(dmnsn_reflection *reflection)
{
  dmnsn_free(reflection);
}

dmnsn_reflection *
dmnsn_new_reflection(void)
{
  dmnsn_reflection *reflection = DMNSN_MALLOC(dmnsn_reflection);
  dmnsn_init_reflection(reflection);
  return reflection;
}

void
dmnsn_init_reflection(dmnsn_reflection *reflection)
{
  reflection->free_fn = dmnsn_default_reflection_free_fn;
  DMNSN_REFCOUNT_INIT(reflection);
}

void
dmnsn_delete_reflection(dmnsn_reflection *reflection)
{
  if (DMNSN_DECREF(reflection)) {
    reflection->free_fn(reflection);
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
  if (!finish->ambient && default_finish->ambient) {
    finish->ambient = default_finish->ambient;
    DMNSN_INCREF(finish->ambient);
  }
  if (!finish->diffuse && default_finish->diffuse) {
    finish->diffuse = default_finish->diffuse;
    DMNSN_INCREF(finish->diffuse);
  }
  if (!finish->specular && default_finish->specular) {
    finish->specular = default_finish->specular;
    DMNSN_INCREF(finish->specular);
  }
  if (!finish->reflection && default_finish->reflection) {
    finish->reflection = default_finish->reflection;
    DMNSN_INCREF(finish->reflection);
  }
}
