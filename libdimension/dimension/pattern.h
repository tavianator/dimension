/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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
 * Patterns
 */

#ifndef DIMENSION_PATTERN_H
#define DIMENSION_PATTERN_H

/* Forward-declare dmnsn_pattern */
typedef struct dmnsn_pattern dmnsn_pattern;

/* Pattern callback */
typedef double dmnsn_pattern_fn(const dmnsn_pattern *pattern, dmnsn_vector v);

/* Generic pattern */
struct dmnsn_pattern {
  /* Callbacks */
  dmnsn_pattern_fn *pattern_fn;
  dmnsn_free_fn    *free_fn;

  /* Transformation matrix */
  dmnsn_matrix trans, trans_inv;

  /* Generic pointer */
  void *ptr;
};

dmnsn_pattern *dmnsn_new_pattern(void);
void dmnsn_delete_pattern(dmnsn_pattern *pattern);

void dmnsn_pattern_init(dmnsn_pattern *pattern);

/* Invoke the pattern callback with the right transformation */
double dmnsn_pattern_value(const dmnsn_pattern *pattern, dmnsn_vector v);

#endif /* DIMENSION_PATTERN_H */
