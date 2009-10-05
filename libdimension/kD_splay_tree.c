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

#include "dimension_impl.h"
#include <stdlib.h>

static dmnsn_kD_splay_node *
dmnsn_new_kD_splay_node()
{
  dmnsn_kD_splay_node *node = malloc(sizeof(dmnsn_kD_splay_node));
  if (!node) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "kD splay tree node allocation failed.");
  }
  return node;
}

static void
dmnsn_delete_kD_splay_node(dmnsn_kD_splay_node *node)
{
  free(node);
}

/* Return an empty tree */
dmnsn_kD_splay_node *
dmnsn_new_kD_splay_tree()
{
  return NULL;
}

/* Copy a kD splay tree */
dmnsn_kD_splay_node *dmnsn_kD_splay_copy(dmnsn_kD_splay_node *root)
{
  dmnsn_kD_splay_node *node = NULL;
  if (root) {
    node = dmnsn_new_kD_splay_node();
    *node = *root;
    node->left  = dmnsn_kD_splay_copy(node->left);
    node->right = dmnsn_kD_splay_copy(node->right);
    node->left->parent  = node;
    node->right->parent = node;
  }
  return node;
}

void
dmnsn_delete_kD_splay_tree(dmnsn_kD_splay_node *root)
{
  if (root) {
    dmnsn_delete_kD_splay_tree(root->left);
    dmnsn_delete_kD_splay_tree(root->right);
    dmnsn_delete_kD_splay_node(root);
  }
}

/* Tree rotations */
static void dmnsn_kD_splay_rotate(dmnsn_kD_splay_node *node);

void
dmnsn_kD_splay(dmnsn_kD_splay_node *node)
{
  while (node->parent) {
    if (!node->parent->parent) {
      /* Zig step - we are a child of the root node */
      dmnsn_kD_splay_rotate(node);
      return;
    } else if ((node == node->parent->left
                && node->parent == node->parent->parent->left)
               || (node == node->parent->right
                   && node->parent == node->parent->parent->right)) {
      /* Zig-zig step - we are a child on the same side as our parent */
      dmnsn_kD_splay_rotate(node->parent);
      dmnsn_kD_splay_rotate(node);
    } else {
      /* Zig-zag step - we are a child on a different side than our parent is */
      dmnsn_kD_splay_rotate(node);
      dmnsn_kD_splay_rotate(node);
    }
  }
}

static void
dmnsn_kD_splay_rotate(dmnsn_kD_splay_node *node)
{
  dmnsn_kD_splay_node *pivot;
  if (node == node->parent->left) {
    /* We are a left child */
    pivot = node->right;

    node->right = node->parent;
    node->right->parent = node;

    node->right->left = pivot;
    pivot->parent = node->right;

    node->parent = node->parent->parent;
  } else {
    /* We are a right child */
    pivot = node->left;

    node->left = node->parent;
    node->left->parent = node;

    node->left->right = pivot;
    pivot->parent = node->left;

    node->parent = node->parent->parent;
  }
}
