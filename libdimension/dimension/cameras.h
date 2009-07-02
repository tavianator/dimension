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

/*
 * Types of cameras.
 */

#ifndef DIMENSION_CAMERAS_H
#define DIMENSION_CAMERAS_H

/* A perspective camera, at the origin, looking at (0, 0, 1).  The feild of view
   is the section of the plane z = 1 from (-0.5, -0.5) to (0.5, 0.5).  Rays are
   transformed by the transformation matrix `trans'. */
dmnsn_camera *dmnsn_new_perspective_camera(dmnsn_matrix trans);
void dmnsn_delete_perspective_camera(dmnsn_camera *camera);

dmnsn_matrix dmnsn_get_perspective_camera_trans(const dmnsn_camera *camera);
void dmnsn_set_perspective_camera_trans(dmnsn_camera *camera, dmnsn_matrix T);

#endif /* DIMENSION_CAMERAS_H */
