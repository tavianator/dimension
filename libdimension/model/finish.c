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

#include "internal.h"
#include "dimension/model.h"

dmnsn_ambient *
dmnsn_new_ambient(dmnsn_pool *pool, dmnsn_color ambient_light)
{
  dmnsn_ambient *ambient = DMNSN_PALLOC(pool, dmnsn_ambient);
  ambient->ambient = ambient_light;
  return ambient;
}

dmnsn_diffuse *
dmnsn_new_diffuse(dmnsn_pool *pool)
{
  dmnsn_diffuse *diffuse = DMNSN_PALLOC(pool, dmnsn_diffuse);
  dmnsn_init_diffuse(diffuse);
  return diffuse;
}

void
dmnsn_init_diffuse(dmnsn_diffuse *diffuse)
{
}

dmnsn_specular *
dmnsn_new_specular(dmnsn_pool *pool)
{
  dmnsn_specular *specular = DMNSN_PALLOC(pool, dmnsn_specular);
  dmnsn_init_specular(specular);
  return specular;
}

void
dmnsn_init_specular(dmnsn_specular *specular)
{
}

dmnsn_reflection *
dmnsn_new_reflection(dmnsn_pool *pool)
{
  dmnsn_reflection *reflection = DMNSN_PALLOC(pool, dmnsn_reflection);
  dmnsn_init_reflection(reflection);
  return reflection;
}

void
dmnsn_init_reflection(dmnsn_reflection *reflection)
{
}

dmnsn_finish
dmnsn_new_finish(void)
{
  dmnsn_finish finish = {
    .ambient = NULL,
    .diffuse = NULL,
    .specular = NULL,
    .reflection = NULL,
  };
  return finish;
}

void
dmnsn_finish_cascade(const dmnsn_finish *default_finish, dmnsn_finish *finish)
{
  if (!finish->ambient) {
    finish->ambient = default_finish->ambient;
  }
  if (!finish->diffuse) {
    finish->diffuse = default_finish->diffuse;
  }
  if (!finish->specular) {
    finish->specular = default_finish->specular;
  }
  if (!finish->reflection) {
    finish->reflection = default_finish->reflection;
  }
}
