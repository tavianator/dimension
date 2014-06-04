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
 * Pre-defined objects.
 */

#include <stdbool.h>

/**
 * A flat triangle, without normal interpolation.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] vertices  The corners of the triangle.
 */
dmnsn_object *dmnsn_new_triangle(dmnsn_pool *pool, dmnsn_vector vertices[3]);

/**
 * A triangle, with normals interpolated between the points.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] vertices  The corners of the triangle.
 * @param[in] normals  The normals at each corner.
 */
dmnsn_object *dmnsn_new_smooth_triangle(dmnsn_pool *pool, dmnsn_vector vertices[3], dmnsn_vector normals[3]);

/**
 * A plane.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] normal  The normal vector of the plane.
 * @return A plane through the origin, with the given normal.
 */
dmnsn_object *dmnsn_new_plane(dmnsn_pool *pool, dmnsn_vector normal);

/**
 * A sphere.
 * @param[in] pool  The memory pool to allocate from.
 * @return A sphere of radius 1, centered at the origin.
 */
dmnsn_object *dmnsn_new_sphere(dmnsn_pool *pool);

/**
 * A cube.
 * @param[in] pool  The memory pool to allocate from.
 * @return An axis-aligned cube, from (-1, -1, -1) to (1, 1, 1).
 */
dmnsn_object *dmnsn_new_cube(dmnsn_pool *pool);

/**
 * A cylinder/cone.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] r1    The bottom radius.
 * @param[in] r2    The top radius.
 * @param[in] open  Whether to render caps.
 * @return A cone slice, from r = \p r1 at y = -1, to r = \p r2 at y = 1
 */
dmnsn_object *dmnsn_new_cone(dmnsn_pool *pool, double r1, double r2, bool open);

/**
 * A torus.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] major  The major radius.
 * @param[in] minor  The minor radius.
 * @return A torus, centered at the origin and lying in the x-z plane.
 */
dmnsn_object *dmnsn_new_torus(dmnsn_pool *pool, double major, double minor);
