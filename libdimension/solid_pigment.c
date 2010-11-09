/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
#include <stdlib.h>

/* Solid color pigment callback */
static dmnsn_color dmnsn_solid_pigment_fn(const dmnsn_pigment *pigment,
                                          dmnsn_vector v);

/* Create a solid color */
dmnsn_pigment *
dmnsn_new_solid_pigment(dmnsn_color color)
{
  dmnsn_pigment *pigment = dmnsn_new_pigment();

  dmnsn_color *solid = dmnsn_malloc(sizeof(dmnsn_color));
  *solid = color;

  pigment->pigment_fn  = &dmnsn_solid_pigment_fn;
  pigment->free_fn     = &dmnsn_free;
  pigment->quick_color = color;
  pigment->ptr         = solid;

  return pigment;
}

/* Solid color callback */
static dmnsn_color
dmnsn_solid_pigment_fn(const dmnsn_pigment *pigment, dmnsn_vector v)
{
  dmnsn_color *color = pigment->ptr;
  return *color;
}
