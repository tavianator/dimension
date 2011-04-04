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
 * Gradient pattern.
 */

#include "dimension.h"

/** Gradient pattern callback. */
static double
dmnsn_gradient_pattern_fn(const dmnsn_pattern *gradient, dmnsn_vector v)
{
  dmnsn_vector *orientation = gradient->ptr;
  double n = fmod(dmnsn_vector_dot(*orientation, v), 1.0);
  if (n < -dmnsn_epsilon)
    n += 1.0;
  return n;
}

dmnsn_pattern *
dmnsn_new_gradient_pattern(dmnsn_vector orientation)
{
  dmnsn_pattern *gradient = dmnsn_new_pattern();

  dmnsn_vector *payload = dmnsn_malloc(sizeof(dmnsn_vector));
  *payload = dmnsn_vector_normalize(orientation);

  gradient->pattern_fn = dmnsn_gradient_pattern_fn;
  gradient->free_fn    = dmnsn_free;
  gradient->ptr        = payload;

  return gradient;
}
