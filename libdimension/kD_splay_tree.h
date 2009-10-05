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

#ifndef DIMENSION_IMPL_KD_SPLAY_TREE_H
#define DIMENSION_IMPL_KD_SPLAY_TREE_H

typedef struct dmnsn_kD_splay_node dmnsn_kD_splay_node;

struct dmnsn_kD_splay_node {
  /* Tree children */
  dmnsn_kD_splay_node *left, *right;

  /* Parent node for easy backtracking */
  dmnsn_kD_splay_node *parent;

  /* Bounding box corners */
  dmnsn_vector min, max;

  /* Node payload */
  dmnsn_object *object;
};

dmnsn_kD_splay_node *dmnsn_new_kD_splay_tree();
dmnsn_kD_splay_node *dmnsn_kD_splay_copy(dmnsn_kD_splay_node *root);
void dmnsn_delete_kD_splay_tree(dmnsn_kD_splay_node *root);

dmnsn_kD_splay_node *dmnsn_kD_splay_insert(dmnsn_kD_splay_node *root,
                                           dmnsn_object *object);

#endif /* DIMENSION_IMPL_KD_SPLAY_TREE_H */
