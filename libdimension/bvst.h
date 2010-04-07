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
 * Bounding Volume Splay Trees for storing object bounding boxes.
 * Each node's bounding box entirely contains the bounding boxes of the nodes
 * to its left, and is entirely contained by the bounding boxes of the nodes to
 * its right.  Splay trees are used for the implementation, to bring commonly-
 * used objects (the most recent object which a ray has hit) near the root of
 * the tree for fast access.  Object's bounding boxes are expanded as needed
 * when inserted into the tree: if they intersect an existing bounding box, they
 * are expanded to contain it.
 */

#ifndef DIMENSION_IMPL_BVST_H
#define DIMENSION_IMPL_BVST_H

typedef struct dmnsn_bvst dmnsn_bvst;
typedef struct dmnsn_bvst_node dmnsn_bvst_node;

struct dmnsn_bvst {
  dmnsn_bvst_node *root;
};

struct dmnsn_bvst_node {
  /* Tree children */
  dmnsn_bvst_node *contains, *container;

  /* Parent node for easy backtracking */
  dmnsn_bvst_node *parent;

  /* Bounding box */
  dmnsn_bounding_box bounding_box;

  /* Node payload */
  dmnsn_object *object;
};

dmnsn_bvst *dmnsn_new_bvst();
dmnsn_bvst *dmnsn_copy_bvst(dmnsn_bvst *tree);
void dmnsn_delete_bvst(dmnsn_bvst *tree);

void dmnsn_bvst_insert(dmnsn_bvst *tree, dmnsn_object *object);
void dmnsn_bvst_splay(dmnsn_bvst *tree, dmnsn_bvst_node *node);

bool dmnsn_bvst_search(dmnsn_bvst *tree, dmnsn_line ray,
                       dmnsn_intersection *intersection);

#endif /* DIMENSION_IMPL_BVST_H */
