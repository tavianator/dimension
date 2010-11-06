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

#include "dimension.h"

/* Allocate a dummy pattern */
dmnsn_pattern *
dmnsn_new_pattern(void)
{
  dmnsn_pattern *pattern = dmnsn_malloc(sizeof(dmnsn_pattern));
  pattern->trans   = dmnsn_identity_matrix();
  pattern->free_fn = NULL;
  return pattern;
}

/* Delete a pattern */
void
dmnsn_delete_pattern(dmnsn_pattern *pattern)
{
  if (pattern->free_fn) {
    (*pattern->free_fn)(pattern->ptr);
  }
  dmnsn_free(pattern);
}

/* Precompute the transformation matrix inverse */
void
dmnsn_pattern_init(dmnsn_pattern *pattern)
{
  pattern->trans_inv = dmnsn_matrix_inverse(pattern->trans);
}

/* Invoke the pattern callback with the right transformation */
double
dmnsn_pattern_value(const dmnsn_pattern *pattern, dmnsn_vector v)
{
  v = dmnsn_transform_vector(pattern->trans_inv, v);
  return (*pattern->pattern_fn)(pattern, v);
}
