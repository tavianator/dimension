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

/*
 * Checker pattern
 */

static double
dmnsn_checker_pattern_fn(const dmnsn_pattern *checker, dmnsn_vector v)
{
  double xmod = fmod(v.x, 2.0);
  double ymod = fmod(v.y, 2.0);
  double zmod = fmod(v.z, 2.0);

  if (xmod < -dmnsn_epsilon)
    xmod += 2.0;
  if (ymod < -dmnsn_epsilon)
    ymod += 2.0;
  if (zmod < -dmnsn_epsilon)
    zmod += 2.0;

  /* Return 0 when an even number of coordinates are in [0, 1), 1 otherwise */
  unsigned int n = 0;
  if (xmod >= 1.0)
    ++n;
  if (ymod >= 1.0)
    ++n;
  if (zmod >= 1.0)
    ++n;
  return (n%2 == 0) ? 0.0 : 1.0;
}

dmnsn_pattern *
dmnsn_new_checker_pattern()
{
  dmnsn_pattern *checker = dmnsn_new_pattern();
  checker->pattern_fn = &dmnsn_checker_pattern_fn;
  return checker;
}