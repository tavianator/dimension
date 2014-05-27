/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

/** Gradient pattern type. */
typedef struct dmnns_gradient {
  dmnsn_pattern pattern;
  dmnsn_vector orientation;
} dmnsn_gradient;

/** Gradient pattern callback. */
static double
dmnsn_gradient_pattern_fn(const dmnsn_pattern *pattern, dmnsn_vector v)
{
  const dmnsn_gradient *gradient = (const dmnsn_gradient *)pattern;
  double n = fmod(dmnsn_vector_dot(gradient->orientation, v), 1.0);
  if (n < -dmnsn_epsilon) {
    n += 1.0;
  }
  return n;
}

dmnsn_pattern *
dmnsn_new_gradient_pattern(dmnsn_vector orientation)
{
  dmnsn_gradient *gradient = DMNSN_MALLOC(dmnsn_gradient);
  gradient->orientation = dmnsn_vector_normalized(orientation);

  dmnsn_pattern *pattern = &gradient->pattern;
  dmnsn_init_pattern(pattern);
  pattern->pattern_fn = dmnsn_gradient_pattern_fn;
  return pattern;
}
