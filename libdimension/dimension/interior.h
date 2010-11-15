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

/**
 * @file
 * Object interiors.
 */

#ifndef DIMENSION_INTERIOR_H
#define DIMENSION_INTERIOR_H

/** An interior */
typedef struct dmnsn_interior {
  double ior; /**< Refractive index. */

  dmnsn_free_fn *free_fn; /**< Destructor callback. */

  /** Generic pointer. */
  void *ptr;

  /** @internal Reference count. */
  unsigned int *refcount;
} dmnsn_interior;

/**
 * Create an interior object.
 * @return The new interior.
 */
dmnsn_interior *dmnsn_new_interior(void);

/**
 * Delete an interior.
 * @param[in,out] interior  The interior to delete.
 */
void dmnsn_delete_interior(dmnsn_interior *interior);

#endif /* DIMENSION_INTERIOR_H */
