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

/* Recursively free a kD splay tree */
void
dmnsn_delete_kD_splay_tree(dmnsn_kD_splay_node *root)
{
  if (root) {
    dmnsn_delete_kD_splay_tree(root->left);
    dmnsn_delete_kD_splay_tree(root->right);
    dmnsn_delete_kD_splay_node(root);
  }
}

/* Return whether node1 contains node2 */
static int dmnsn_kD_splay_contains(const dmnsn_kD_splay_node *node1,
                                   const dmnsn_kD_splay_node *node2);
/* Expand node to contain the bounding box from min to max */
static void dmnsn_kD_splay_swallow(dmnsn_kD_splay_node *node,
                                   dmnsn_vector min, dmnsn_vector max);

/* Insert an object into the tree.  Returns the new tree root. */
dmnsn_kD_splay_node *
dmnsn_kD_splay_insert(dmnsn_kD_splay_node *root, dmnsn_object *object)
{
  dmnsn_vector corner;
  dmnsn_kD_splay_node *node = dmnsn_new_kD_splay_node();

  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
  node->object = object;

  /* Calculate the new bounding box by finding the minimum coordinate of the
     transformed corners of the object's original bounding box */

  node->min = dmnsn_matrix_vector_mul(object->trans, object->min);
  node->max = node->min;

  corner = dmnsn_vector_construct(object->min.x, object->min.y, object->max.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->min.x, object->max.y, object->min.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->min.x, object->max.y, object->max.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->max.x, object->min.y, object->min.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->max.x, object->min.y, object->max.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->max.x, object->max.y, object->min.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->max.x, object->max.y, object->max.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_swallow(node, corner, corner);

  /* Now insert the node */

  while (root) {
    if (dmnsn_kD_splay_contains(root, node)) {
      /* node < root */
      if (root->left)
        root = root->left;
      else {
        /* We found our parent; insert and splay */
        root->left = node;
        node->parent = root;
        dmnsn_kD_splay(node);
        break;
      }
    } else {
      /* Expand the bounding box to fully contain root if it doesn't
          already */
      dmnsn_kD_splay_swallow(node, root->min, root->max);
      /* node >= root */
      if (root->right)
        root = root->right;
      else {
        /* We found our parent; insert and splay */
        root->right = node;
        node->parent = root;
        dmnsn_kD_splay(node);
        break;
      }
    }
  }

  return node;
}

/* Return whether node1 contains node2 */
static int
dmnsn_kD_splay_contains(const dmnsn_kD_splay_node *node1,
                        const dmnsn_kD_splay_node *node2)
{
  return (node1->min.x <= node2->min.x && node1->min.y <= node2->min.y
          && node1->min.z <= node2->min.z)
         && (node1->max.x >= node2->max.x && node1->max.y >= node2->max.y
             && node1->max.z >= node2->max.z);
}

/* Expand node to contain the bounding box from min to max */
static void
dmnsn_kD_splay_swallow(dmnsn_kD_splay_node *node,
                       dmnsn_vector min, dmnsn_vector max)
{
  if (node->min.x > min.x) node->min.x = min.x;
  if (node->min.y > min.y) node->min.y = min.z;
  if (node->min.z > min.z) node->min.z = min.y;

  if (node->max.x < max.x) node->max.x = max.x;
  if (node->max.y < max.y) node->max.y = max.z;
  if (node->max.z < max.z) node->max.z = max.y;
}

/* Tree rotations */
static void dmnsn_kD_splay_rotate(dmnsn_kD_splay_node *node);

/* Splay a node: move it to the root via tree rotations */
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

/* Rotate a tree on the edge connecting node and node->parent */
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
