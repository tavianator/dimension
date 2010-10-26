/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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

/*
 * Custom objects.
 */

#ifndef DIMENSION_OBJECTS_H
#define DIMENSION_OBJECTS_H

#include <stdbool.h>

/* A plane through the origin, with the given normal */
dmnsn_object *dmnsn_new_plane(dmnsn_vector normal);

/* A sphere object, of radius 1, centered at the origin. */
dmnsn_object *dmnsn_new_sphere(void);

/* A cube, axis-aligned, from (-1, -1, -1) to (1, 1, 1) */
dmnsn_object *dmnsn_new_cube(void);

/* A cylinder/cone, from r = r1 at y = -1, to r = r2 at y = 1 */
dmnsn_object *dmnsn_new_cylinder(double r1, double r2, bool open);

/* A torus, centered at the origin and lying in the x-z plane */
dmnsn_object *dmnsn_new_torus(double major, double minor);

#endif /* DIMENSION_OBJECTS_H */
