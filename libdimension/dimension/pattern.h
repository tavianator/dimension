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
 * Patterns.  Patterns are functions which map vectors to scalars, which are
 * used for pigments and normals.
 */

#ifndef DIMENSION_PATTERN_H
#define DIMENSION_PATTERN_H

/* Forward-declare dmnsn_pattern */
typedef struct dmnsn_pattern dmnsn_pattern;

/**
 * Pattern callback.
 * @param[in] pattern  The pattern itself.
 * @param[in] v        The point at which to evaluate the pattern.
 * @return The value of the pattern at \p v.
 */
typedef double dmnsn_pattern_fn(const dmnsn_pattern *pattern, dmnsn_vector v);

/** A pattern. */
struct dmnsn_pattern {
  dmnsn_pattern_fn *pattern_fn; /**< The pattern callback. */
  dmnsn_free_fn    *free_fn;    /**< The destructor callback. */

  dmnsn_matrix trans;     /**< The transformation matrix of the pattern. */
  dmnsn_matrix trans_inv; /**< The inverse of the transformation matrix. */

  void *ptr; /**< Generic pointer. */
};

/**
 * Allocate an dummy pattern.
 * @return A pattern with no callbacks set.
 */
dmnsn_pattern *dmnsn_new_pattern(void);

/**
 * Delete a pattern.
 * @param[in,out] pattern  The pattern to destroy.
 */
void dmnsn_delete_pattern(dmnsn_pattern *pattern);

/**
 * Initialize a pattern.  This precomputes some values that are used during
 * ray-tracing; the pattern will not work until it has been initialized, but
 * should not be modified after it has been initialized.  Patterns are generally
 * initialized for you.
 * @param[in,out] pattern  The pattern to initialize.
 */
void dmnsn_initialize_pattern(dmnsn_pattern *pattern);

/**
 * Invoke the pattern callback with the right transformation.
 * @param[in] pattern  The pattern to evaluate.
 * @param[in] v        The point to get the pattern value for.
 * @return The value of the pattern at \p v.
 */
double dmnsn_pattern_value(const dmnsn_pattern *pattern, dmnsn_vector v);

#endif /* DIMENSION_PATTERN_H */
