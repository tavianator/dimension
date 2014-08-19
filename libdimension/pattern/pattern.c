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
 * Patterns.
 */

#include "dimension/pattern.h"

dmnsn_pattern *
dmnsn_new_pattern(dmnsn_pool *pool)
{
  dmnsn_pattern *pattern = DMNSN_PALLOC(pool, dmnsn_pattern);
  dmnsn_init_pattern(pattern);
  return pattern;
}

void
dmnsn_init_pattern(dmnsn_pattern *pattern)
{
}

double
dmnsn_pattern_value(const dmnsn_pattern *pattern, dmnsn_vector v)
{
  return pattern->pattern_fn(pattern, v);
}
