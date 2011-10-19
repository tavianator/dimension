/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Object pigments.
 */

/* Forward-declare dmnsn_pigment */
typedef struct dmnsn_pigment dmnsn_pigment;

/**
 * Pigment callback.
 * @param[in] pigment  The pigment itself.
 * @param[in] v        The point to color.
 * @return The color of the pigment at \p v.
 */
typedef dmnsn_color dmnsn_pigment_fn(const dmnsn_pigment *pigment,
                                     dmnsn_vector v);

/**
 * Pigment initializer callback.
 * @param[in,out] pigment  The pigment to initialize.
 */
typedef void dmnsn_pigment_initialize_fn(dmnsn_pigment *pigment);

/** A pigment. */
struct dmnsn_pigment {
  dmnsn_pigment_fn *pigment_fn;               /**< The pigment callback. */
  dmnsn_pigment_initialize_fn *initialize_fn; /**< The initializer callback. */
  dmnsn_free_fn *free_fn;                     /**< The destructor callback. */

  dmnsn_matrix trans;     /**< Transformation matrix. */
  dmnsn_matrix trans_inv; /**< The inverse of the transformation matrix. */

  /** Quick color -- used for low-quality renders. */
  dmnsn_color quick_color;

  /** Generic pointer. */
  void *ptr;

  dmnsn_refcount refcount; /** @internal Reference count. */
  bool initialized; /** @internal Whether the pigment is initialized. */
};

/**
 * Allocate a new dummy pigment.
 * @return The allocated pigment.
 */
dmnsn_pigment *dmnsn_new_pigment(void);

/**
 * Delete a pigment.
 * @param[in,out] pigment  The pigment to delete.
 */
void dmnsn_delete_pigment(dmnsn_pigment *pigment);

/**
 * Initialize a pigment.  Pigments should not be used before being initialized,
 * but should not be modified after being initialized.  Pigments are generally
 * initialized for you.
 * @param[in,out] pigment  The pigment to initialize.
 */
void dmnsn_pigment_initialize(dmnsn_pigment *pigment);

/**
 * Evaluate the color of a pigment at a point.
 * @param[in] pigment  The pigment to evaluate.
 * @param[in] v        The point to color.
 * @return The color at \p v.
 */
dmnsn_color dmnsn_pigment_evaluate(const dmnsn_pigment *pigment,
                                   dmnsn_vector v);
