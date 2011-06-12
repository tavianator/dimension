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
 * Sky spheres.
 */

#include "dimension.h"

dmnsn_sky_sphere *
dmnsn_new_sky_sphere(void)
{
  dmnsn_sky_sphere *sky_sphere = dmnsn_malloc(sizeof(dmnsn_sky_sphere));
  sky_sphere->pigments = dmnsn_new_array(sizeof(dmnsn_pigment *));
  sky_sphere->trans    = dmnsn_identity_matrix();
  sky_sphere->refcount = 1;
  return sky_sphere;
}

void
dmnsn_delete_sky_sphere(dmnsn_sky_sphere *sky_sphere)
{
  if (DMNSN_DECREF(sky_sphere)) {
    DMNSN_ARRAY_FOREACH (dmnsn_pigment **, pigment, sky_sphere->pigments) {
      dmnsn_delete_pigment(*pigment);
    }
    dmnsn_delete_array(sky_sphere->pigments);
    dmnsn_free(sky_sphere);
  }
}

void
dmnsn_initialize_sky_sphere(dmnsn_sky_sphere *sky_sphere)
{
  DMNSN_ARRAY_FOREACH (dmnsn_pigment **, pigment, sky_sphere->pigments) {
    (*pigment)->trans = dmnsn_matrix_mul(sky_sphere->trans, (*pigment)->trans);
    dmnsn_initialize_pigment(*pigment);
  }
}

dmnsn_color
dmnsn_sky_sphere_color(const dmnsn_sky_sphere *sky_sphere, dmnsn_vector d)
{
  dmnsn_color color = dmnsn_clear;

  DMNSN_ARRAY_FOREACH (const dmnsn_pigment **, pigment, sky_sphere->pigments) {
    dmnsn_color sky = dmnsn_evaluate_pigment(*pigment, d);
    color = dmnsn_apply_filter(color, sky);
  }

  return color;
}
