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
 * Lines in 3-D space.
 */

#ifndef DMNSN_MATH_H
#error "Please include <dimension/math.h> instead of this header directly."
#endif

/** A line, or ray. */
typedef struct dmnsn_ray {
  dmnsn_vector x0; /**< The origin of the ray. */
  dmnsn_vector n;  /**< The direction of the ray. */
} dmnsn_ray;

/** A standard format string for rays. */
#define DMNSN_RAY_FORMAT "(<%g, %g, %g> + t*<%g, %g, %g>)"
/** The appropriate arguements to printf() a ray. */
#define DMNSN_RAY_PRINTF(l)                                     \
  DMNSN_VECTOR_PRINTF((l).x0), DMNSN_VECTOR_PRINTF((l).n)

/**
 * Construct a new ray.
 * @param[in] x0  The origin of the ray.
 * @param[in] n   The direction of the ray.
 * @return The new ray.
 */
DMNSN_INLINE dmnsn_ray
dmnsn_new_ray(dmnsn_vector x0, dmnsn_vector n)
{
  dmnsn_ray l = { x0, n };
  return l;
}

/**
 * Return the point at \p t on a ray.
 * The point is defined by \f$ l.\vec{x_0} + t \cdot l.\vec{n} \f$
 */
DMNSN_INLINE dmnsn_vector
dmnsn_ray_point(dmnsn_ray l, double t)
{
  return dmnsn_vector_add(l.x0, dmnsn_vector_mul(t, l.n));
}

/** Add epsilon*l.n to l.x0, to avoid self-intersections. */
DMNSN_INLINE dmnsn_ray
dmnsn_ray_add_epsilon(dmnsn_ray l)
{
  return dmnsn_new_ray(
    dmnsn_vector_add(
      l.x0,
      dmnsn_vector_mul(1.0e3*dmnsn_epsilon, l.n)
    ),
    l.n
  );
}

/** Return whether a ray contains any NaN entries. */
DMNSN_INLINE bool
dmnsn_ray_isnan(dmnsn_ray l)
{
  return dmnsn_vector_isnan(l.x0) || dmnsn_vector_isnan(l.n);
}
