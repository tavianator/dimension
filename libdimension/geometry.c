/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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

dmnsn_vector
dmnsn_vector_construct(dmnsn_scalar x, dmnsn_scalar y, dmnsn_scalar z)
{
  dmnsn_vector v = { .x = x, .y = y, .z = z };
  return v;
}

dmnsn_vector
dmnsn_vector_add(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { .x = lhs.x + rhs.x,
                     .y = lhs.y + rhs.y,
                     .z = lhs.z + rhs.z };
  return v;
}

dmnsn_vector
dmnsn_vector_sub(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { .x = lhs.x - rhs.x,
                     .y = lhs.y - rhs.y,
                     .z = lhs.z - rhs.z };
  return v;
}

dmnsn_vector
dmnsn_vector_mul(dmnsn_scalar lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { .x = lhs*rhs.x, .y = lhs*rhs.y, .z = lhs*rhs.z };
  return v;
}

dmnsn_vector
dmnsn_vector_div(dmnsn_vector lhs, dmnsn_scalar rhs)
{
  dmnsn_vector v = { .x = lhs.x/rhs, .y = lhs.y/rhs, .z = lhs.z/rhs };
  return v;
}

dmnsn_scalar
dmnsn_vector_dot(dmnsn_vector lhs, dmnsn_vector rhs)
{
  return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

dmnsn_vector
dmnsn_vector_cross(dmnsn_vector lhs, dmnsn_vector rhs)
{
  dmnsn_vector v = { .x = lhs.y*rhs.z - lhs.z*rhs.y,
                     .y = lhs.z*rhs.x - lhs.x*rhs.z,
                     .z = lhs.x*rhs.y - lhs.y*rhs.x };
  return v;
}

dmnsn_vector
dmnsn_line_point(dmnsn_line l, dmnsn_scalar t)
{
  return dmnsn_vector_add(l.x0, dmnsn_vector_mul(t, l.n));
}
