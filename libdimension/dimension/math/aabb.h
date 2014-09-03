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
 * Axis-aligned bounding boxes.
 */

#ifndef DMNSN_MATH_H
#error "Please include <dimension/math.h> instead of this header directly."
#endif

/** An axis-aligned bounding box. */
typedef struct dmnsn_aabb {
  dmnsn_vector min; /**< The coordinate-wise minimum extent of the box. */
  dmnsn_vector max; /**< The coordinate-wise maximum extent of the box. */
} dmnsn_aabb;

/** A standard format string for bounding boxes. */
#define DMNSN_AABB_FORMAT "(<%g, %g, %g> ==> <%g, %g, %g>)"
/** The appropriate arguements to printf() a bounding box. */
#define DMNSN_AABB_PRINTF(box)                                          \
  DMNSN_VECTOR_PRINTF((box).min), DMNSN_VECTOR_PRINTF((box).max)

/**
 * Construct a new bounding box.
 * @param[in] min  The minimal extent of the bounding box.
 * @param[in] max  The maximal extent of the bounding box.
 * @return The new bounding box.
 */
DMNSN_INLINE dmnsn_aabb
dmnsn_new_aabb(dmnsn_vector min, dmnsn_vector max)
{
  dmnsn_aabb box = { min, max };
  return box;
}

/** Return the bounding box which contains nothing. */
DMNSN_INLINE dmnsn_aabb
dmnsn_zero_aabb(void)
{
  dmnsn_aabb box = {
    { {  DMNSN_INFINITY,  DMNSN_INFINITY,  DMNSN_INFINITY } },
    { { -DMNSN_INFINITY, -DMNSN_INFINITY, -DMNSN_INFINITY } }
  };
  return box;
}

/** Return the bounding box which contains everything. */
DMNSN_INLINE dmnsn_aabb
dmnsn_infinite_aabb(void)
{
  dmnsn_aabb box = {
    { { -DMNSN_INFINITY, -DMNSN_INFINITY, -DMNSN_INFINITY } },
    { {  DMNSN_INFINITY,  DMNSN_INFINITY,  DMNSN_INFINITY } }
  };
  return box;
}

/**
 * Construct a new symmetric bounding box.
 * @param[in] r  The extent of the bounding box from the origin.
 * @return The new bounding box.
 */
DMNSN_INLINE dmnsn_aabb
dmnsn_symmetric_aabb(dmnsn_vector r)
{
  dmnsn_vector minus_r = dmnsn_vector_negate(r);
  dmnsn_aabb box = {
    dmnsn_vector_min(r, minus_r),
    dmnsn_vector_max(r, minus_r)
  };
  return box;
}

/** Return whether \p p is within the axis-aligned bounding box. */
DMNSN_INLINE bool
dmnsn_aabb_contains(dmnsn_aabb box, dmnsn_vector p)
{
  for (unsigned int i = 0; i < 3; ++i) {
    if (p.n[i] < box.min.n[i]) {
      return false;
    }
  }

  for (unsigned int i = 0; i < 3; ++i) {
    if (p.n[i] > box.max.n[i]) {
      return false;
    }
  }

  return true;
}

/** Return whether a bounding box is infinite. */
DMNSN_INLINE bool
dmnsn_aabb_is_infinite(dmnsn_aabb box)
{
  return box.min.n[0] == -DMNSN_INFINITY;
}

/**
 * Expand a bounding box to contain a point
 * @param[in] box  The bounding box to expand.
 * @param[in] point  The point to swallow.
 * @return The expanded bounding box.
 */
DMNSN_INLINE dmnsn_aabb
dmnsn_aabb_swallow(dmnsn_aabb box, dmnsn_vector point)
{
  dmnsn_aabb ret = {
    dmnsn_vector_min(box.min, point),
    dmnsn_vector_max(box.max, point)
  };
  return ret;
}

/** Return whether a bounding box has any NaN components. */
DMNSN_INLINE bool
dmnsn_aabb_isnan(dmnsn_aabb box)
{
  return dmnsn_vector_isnan(box.min) || dmnsn_vector_isnan(box.max);
}
