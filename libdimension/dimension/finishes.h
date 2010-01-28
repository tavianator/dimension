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

/*
 * Custom finishes.
 */

#ifndef DIMENSION_FINISHES_H
#define DIMENSION_FINISHES_H

/* Add two finishes */
dmnsn_finish *dmnsn_new_finish_combination(dmnsn_finish *f1, dmnsn_finish *f2);

dmnsn_finish *dmnsn_new_ambient_finish(dmnsn_color ambient);
dmnsn_finish *dmnsn_new_diffuse_finish(double diffuse);

/* A phong specular highlight */
dmnsn_finish *dmnsn_new_phong_finish(double specular, double exp);

/* Specular reflection */
dmnsn_finish *dmnsn_new_reflective_finish(dmnsn_color min, dmnsn_color max,
                                          double falloff);

#endif /* DIMENSION_FINISHES_H */
