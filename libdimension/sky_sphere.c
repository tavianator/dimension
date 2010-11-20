/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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
dmnsn_new_sky_sphere()
{
  dmnsn_sky_sphere *sky_sphere = dmnsn_malloc(sizeof(dmnsn_sky_sphere));
  sky_sphere->pigments = dmnsn_new_array(sizeof(dmnsn_pigment *));
  sky_sphere->trans = dmnsn_identity_matrix();
  return sky_sphere;
}

void
dmnsn_delete_sky_sphere(dmnsn_sky_sphere *sky_sphere)
{
  if (sky_sphere) {
    DMNSN_ARRAY_FOREACH (dmnsn_pigment **, pigment, sky_sphere->pigments) {
      dmnsn_delete_pigment(*pigment);
    }
    dmnsn_free(sky_sphere);
  }
}

void
dmnsn_sky_sphere_init(dmnsn_sky_sphere *sky_sphere)
{
  DMNSN_ARRAY_FOREACH (dmnsn_pigment **, pigment, sky_sphere->pigments) {
    (*pigment)->trans = dmnsn_matrix_mul(sky_sphere->trans, (*pigment)->trans);
    dmnsn_pigment_init(*pigment);
  }
}

dmnsn_color
dmnsn_sky_sphere_color(const dmnsn_sky_sphere *sky_sphere, dmnsn_vector d)
{
  dmnsn_color color = dmnsn_black;
  color.trans = 1.0;

  DMNSN_ARRAY_FOREACH (const dmnsn_pigment **, pigment, sky_sphere->pigments) {
    dmnsn_pigment_fn *pigment_fn = (*pigment)->pigment_fn;
    if (pigment_fn) {
      dmnsn_color sky = (*pigment_fn)(*pigment, d);
      color = dmnsn_color_add(dmnsn_color_filter(color, sky), sky);
    }
  }

  return color;
}
