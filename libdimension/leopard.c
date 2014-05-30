/*************************************************************************
 * Copyright (C) 2011-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Leopard pattern.
 */

#include "dimension-internal.h"
#include <math.h>

/** Leopard pattern callback. */
static double
dmnsn_leopard_pattern_fn(const dmnsn_pattern *leopard, dmnsn_vector v)
{
  double val = (sin(v.x) + sin(v.y) + sin(v.z))/3.0;
  return val*val;
}

/** The singleton instance. */
static dmnsn_pattern dmnsn_leopard_instance = {
  .pattern_fn = dmnsn_leopard_pattern_fn,
};

dmnsn_pattern *
dmnsn_new_leopard_pattern(dmnsn_pool *pool)
{
  return &dmnsn_leopard_instance;
}
