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

#include <stdbool.h>

/** A priority R-tree; the spatial subdivision method used for intersection
    queries against the scene graph. */
typedef struct dmnsn_prtree {
  dmnsn_bounding_box bounding_box; /**< The bounding box for the whole scene. */
  dmnsn_array *unbounded;          /**< The unbounded objects. */
  dmnsn_array *bounded;            /**< A PR-tree of the bounded objects. */
  size_t id;                       /**< A unique ID for the PR-tree. */
} dmnsn_prtree;

/** Create a PR-tree. */
DMNSN_INTERNAL dmnsn_prtree *dmnsn_new_prtree(const dmnsn_array *objects);
/** Delete a PR-tree. */
DMNSN_INTERNAL void dmnsn_delete_prtree(dmnsn_prtree *tree);

/** Find the closest ray-object intersection in the tree. */
DMNSN_INTERNAL bool dmnsn_prtree_intersection(const dmnsn_prtree *tree,
                                              dmnsn_line ray,
                                              dmnsn_intersection *intersection,
                                              bool reset);
/** Determine whether a point is inside any object in the tree. */
DMNSN_INTERNAL bool dmnsn_prtree_inside(const dmnsn_prtree *tree,
                                        dmnsn_vector point);
