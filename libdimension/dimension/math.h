/*************************************************************************
 * Copyright (C) 2014 Tavian Barnes <tavianator@tavianator.com>          *
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
 * Mathematical functions and types.
 */

#ifndef DMNSN_MATH_H
#define DMNSN_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <dimension/base.h>

#include <dimension/math/scalar.h>
#include <dimension/math/vector.h>
#include <dimension/math/ray.h>
#include <dimension/math/aabb.h>
#include <dimension/math/matrix.h>

#ifdef __cplusplus
}
#endif

#endif /* DMNSN_MATH_H */

#if defined(DMNSN_SHORT_NAMES) && !defined(DMNSN_SHORT_NAMES_DEFINED)
  #define DMNSN_SHORT_NAMES_DEFINED

  /** Short name for the \a x component of a \c dmnsn_vector. */
  #define X n[0]
  /** Short name for the \a y component of a \c dmnsn_vector. */
  #define Y n[1]
  /** Short name for the \a z component of a \c dmnsn_vector. */
  #define Z n[2]
#endif
