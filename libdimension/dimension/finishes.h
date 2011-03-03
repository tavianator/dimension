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
 * Pre-defined finishes.
 */

#ifndef DIMENSION_FINISHES_H
#define DIMENSION_FINISHES_H

/**
 * Add two finishes together.
 * @param[in,out] f1  The first finish.
 * @param[in,out] f2  The second finish.
 * @return A finish that adds the values of two finishes together.
 */
dmnsn_finish *dmnsn_new_finish_combination(dmnsn_finish *f1, dmnsn_finish *f2);

/**
 * Ambient finish.
 * @param[in] ambient  The color of the ambient light.
 * @return A finish with ambient light.
 */
dmnsn_finish *dmnsn_new_ambient_finish(dmnsn_color ambient);

/**
 * Diffuse finish.
 * @param[in] diffuse  The diffuse reflection coefficient.
 * @return A finish with diffuse reflection.
 */
dmnsn_finish *dmnsn_new_diffuse_finish(double diffuse);

/**
 * A phong specular highlight.
 * @param[in] specular  The specular reflection coefficient.
 * @param[in] exp       The exponent (roughly the highlight size).
 * @return A finish with phong specular highlight.
 */
dmnsn_finish *dmnsn_new_phong_finish(double specular, double exp);

/**
 * Specular (mirror) reflection.
 * @param[in] min      Reflection at paralell angles.
 * @param[in] max      Reflection at perpendicular angles (often == \p min).
 * @param[in] falloff  Degree of exponential falloff (usually 1).
 * @return A finish with specular reflection.
 */
dmnsn_finish *dmnsn_new_reflective_finish(dmnsn_color min, dmnsn_color max,
                                          double falloff);

#endif /* DIMENSION_FINISHES_H */
