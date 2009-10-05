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
    node->left = dmnsn_kD_splay_copy(node->left);
    node->right = dmnsn_kD_splay_copy(node->right);
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
