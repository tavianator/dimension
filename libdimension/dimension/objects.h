/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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

#ifndef DIMENSION_OBJECTS_H
#define DIMENSION_OBJECTS_H

#include <stdbool.h>

/**
 * A plane.
 * @param[in] normal  The normal vector of the plane.
 * @return A plane through the origin, with the given normal.
 */
dmnsn_object *dmnsn_new_plane(dmnsn_vector normal);

/**
 * A sphere.
 * @return A sphere of radius 1, centered at the origin.
 */
dmnsn_object *dmnsn_new_sphere(void);

/**
 * A cube.
 * @return An axis-aligned cube, from (-1, -1, -1) to (1, 1, 1).
 */
dmnsn_object *dmnsn_new_cube(void);

/**
 * A cylinder/cone.
 * @param[in] r1    The bottom radius.
 * @param[in] r2    The top radius.
 * @param[in] open  Whether to render caps.
 * @return A cone slice, from r = \p r1 at y = -1, to r = \p r2 at y = 1
 */
dmnsn_object *dmnsn_new_cone(double r1, double r2, bool open);

/**
 * A torus.
 * @param[in] major  The major radius.
 * @param[in] minor  The minor radius.
 * @return A torus, centered at the origin and lying in the x-z plane.
 */
dmnsn_object *dmnsn_new_torus(double major, double minor);

#endif /* DIMENSION_OBJECTS_H */
