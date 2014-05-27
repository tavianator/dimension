/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

/** An interior. */
typedef struct dmnsn_interior {
  double ior; /**< Refractive index. */

  DMNSN_REFCOUNT; /**< Reference count. */
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

/**
 * Fill missing interior properties from a default interior.
 * @param[in]     default_interior  The default interior.
 * @param[in,out] interiorp         A pointer to the interior to fill.
 */
void dmnsn_interior_cascade(dmnsn_interior *default_interior,
                            dmnsn_interior **interiorp);
