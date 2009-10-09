/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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
 * k-dimensional (in this case, k == 3) trees for storing object bounding boxes.
 * Each node's bounding box entirely contains the bounding boxes of the nodes
 * to its left, and is entirely contained by the bounding boxes of the nodes to
 * its right.  Splay trees are used for the implementation, to bring commonly-
 * used objects (the most recent object which a ray has hit) near the root of
 * the tree for fast access.  Object's bounding boxes are expanded as needed
 * when inserted into the tree: if they intersect an existing bounding box, they
 * are expanded to contain it.
 */

#ifndef DIMENSION_IMPL_KD_SPLAY_TREE_H
#define DIMENSION_IMPL_KD_SPLAY_TREE_H

typedef struct dmnsn_kD_splay_tree dmnsn_kD_splay_tree;
typedef struct dmnsn_kD_splay_node dmnsn_kD_splay_node;

struct dmnsn_kD_splay_tree {
  dmnsn_kD_splay_node *root;
};

struct dmnsn_kD_splay_node {
  /* Tree children */
  dmnsn_kD_splay_node *contains, *container;

  /* Parent node for easy backtracking */
  dmnsn_kD_splay_node *parent;

  /* Bounding box corners */
  dmnsn_vector min, max;

  /* Node payload */
  dmnsn_object *object;
};

dmnsn_kD_splay_tree *dmnsn_new_kD_splay_tree();
dmnsn_kD_splay_tree *dmnsn_kD_splay_copy(dmnsn_kD_splay_tree *tree);
void dmnsn_delete_kD_splay_tree(dmnsn_kD_splay_tree *tree);

void dmnsn_kD_splay_insert(dmnsn_kD_splay_tree *tree, dmnsn_object *object);
void dmnsn_kD_splay(dmnsn_kD_splay_tree *tree, dmnsn_kD_splay_node *node);

dmnsn_intersection *dmnsn_kD_splay_search(dmnsn_kD_splay_tree *tree,
                                          dmnsn_line ray);

#endif /* DIMENSION_IMPL_KD_SPLAY_TREE_H */
