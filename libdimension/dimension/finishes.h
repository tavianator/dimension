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
 * Pre-defined finishes.
 */

/**
 * Regular diffuse finish.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] diffuse  The diffuse reflection coefficient.
 * @return A diffuse finish component.
 */
dmnsn_diffuse *dmnsn_new_lambertian(dmnsn_pool *pool, double diffuse);

/**
 * A phong specular highlight.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] specular  The specular reflection coefficient.
 * @param[in] exp  The exponent (roughly the highlight size).
 * @return A phong specular finish component.
 */
dmnsn_specular *dmnsn_new_phong(dmnsn_pool *pool, double specular, double exp);

/**
 * Specular (mirror) reflection.
 * @param[in] pool  The memory pool to allocate from.
 * @param[in] min  Reflection at paralell angles.
 * @param[in] max  Reflection at perpendicular angles (often == \p min).
 * @param[in] falloff  Degree of exponential falloff (usually 1).
 * @return A reflective finish component.
 */
dmnsn_reflection *dmnsn_new_basic_reflection(dmnsn_pool *pool, dmnsn_color min, dmnsn_color max, double falloff);
