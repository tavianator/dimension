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

/*
 * Priority R-Trees for storing bounding box hierarchies.  PR-trees are a data
 * structure introduced by Arge, de Berg, Haverkort, and Yi, which provides
 * asymptotically optimal worst-case lookup, while remaining efficient with
 * real-world data.  Their structure is derived from B-trees.
 */

#ifndef DIMENSION_IMPL_PRTREE_H
#define DIMENSION_IMPL_PRTREE_H

#include <stdbool.h>

/* Number of children per node */
#define DMNSN_PRTREE_B 6

typedef struct dmnsn_prtree_node {
  dmnsn_bounding_box bounding_box;

  /* Children (objects or subtrees) */
  bool is_leaf;
  void *children[DMNSN_PRTREE_B];
  dmnsn_bounding_box bounding_boxes[DMNSN_PRTREE_B];
} dmnsn_prtree_node;

typedef struct dmnsn_prtree {
  dmnsn_bounding_box bounding_box;
  dmnsn_prtree_node *root;
  dmnsn_array *unbounded;
} dmnsn_prtree;

dmnsn_prtree *dmnsn_new_prtree(const dmnsn_array *objects);
void dmnsn_delete_prtree(dmnsn_prtree *tree);

bool dmnsn_prtree_intersection(const dmnsn_prtree *tree, dmnsn_line ray,
                               dmnsn_intersection *intersection);
bool dmnsn_prtree_inside(const dmnsn_prtree *tree, dmnsn_vector point);

#endif /* DIMENSION_IMPL_PRTREE_H */
