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
 * Pre-defined patterns.
 */

#ifndef DIMENSION_PATTERNS_H
#define DIMENSION_PATTERNS_H

/**
 * A checker pattern.  The pattern is composed of tesselating unit cubes
 * alternating between 0 and 1.
 * @return A checker pattern.
 */
dmnsn_pattern *dmnsn_new_checker_pattern(void);

/**
 * A gradient.  The value starts at 0 at the origin, and goes linearly to 1 in
 * the direction of \p orientation, then repeats after a distance of 1.
 * @param[in] orientation  The direction of the gradient.
 * @return A gradient pattern.
 */
dmnsn_pattern *dmnsn_new_gradient_pattern(dmnsn_vector orientation);

#endif /* DIMENSION_PATTERNS_H */
