/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * @file.
 * Priority R-trees.  PR-trees are a data structure introduced by Arge, de Berg,
 * Haverkort, and Yi, which provides asymptotically optimal worst-case lookup,
 * while remaining efficient with real-world data.  Their structure is derived
 * from B-trees.
 */

#ifndef DIMENSION_IMPL_PRTREE_H
#define DIMENSION_IMPL_PRTREE_H

#include <stdbool.h>

typedef struct dmnsn_prtree {
  dmnsn_bounding_box bounding_box;
  dmnsn_array *unbounded;
  dmnsn_array *bounded;
} dmnsn_prtree;

/** Create a PR-tree. */
dmnsn_prtree *dmnsn_new_prtree(const dmnsn_array *objects);
/** Delete a PR-tree. */
void dmnsn_delete_prtree(dmnsn_prtree *tree);

/** Find the closest ray-object intersection in the tree. */
bool dmnsn_prtree_intersection(const dmnsn_prtree *tree, dmnsn_line ray,
                               dmnsn_intersection *intersection);
/** Determine whether a point is inside any object in the tree. */
bool dmnsn_prtree_inside(const dmnsn_prtree *tree, dmnsn_vector point);

#endif /* DIMENSION_IMPL_PRTREE_H */
