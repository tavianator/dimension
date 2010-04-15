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

#include "dimension_impl.h"
#include <stdlib.h>

/* Return an empty tree */
dmnsn_bvst *
dmnsn_new_bvst()
{
  dmnsn_bvst *tree = dmnsn_malloc(sizeof(dmnsn_bvst));
  tree->root = NULL;
  return tree;
}

static dmnsn_bvst_node *
dmnsn_new_bvst_node()
{
  dmnsn_bvst_node *node = dmnsn_malloc(sizeof(dmnsn_bvst_node));
  return node;
}

/* Recursively copy the nodes of a BVST */
static dmnsn_bvst_node *
dmnsn_bvst_copy_recursive(dmnsn_bvst_node *root)
{
  dmnsn_bvst_node *node = dmnsn_new_bvst_node();
  *node = *root;
  if (node->contains) {
    node->contains = dmnsn_bvst_copy_recursive(node->contains);
    node->contains->parent = node;
  }
  if (node->container) {
    node->container = dmnsn_bvst_copy_recursive(node->container);
    node->container->parent = node;
  }
  return node;
}

/* Copy a BVST */
dmnsn_bvst *
dmnsn_copy_bvst(dmnsn_bvst *tree)
{
  dmnsn_bvst *copy = dmnsn_new_bvst();
  if (tree->root)
    copy->root = dmnsn_bvst_copy_recursive(tree->root);
  else
    copy->root = NULL;
  return copy;
}

static void
dmnsn_delete_bvst_node(dmnsn_bvst_node *node)
{
  free(node);
}

/* Recursively free a BVST */
static void
dmnsn_delete_bvst_recursive(dmnsn_bvst_node *node)
{
  if (node) {
    dmnsn_delete_bvst_recursive(node->contains);
    dmnsn_delete_bvst_recursive(node->container);
    dmnsn_delete_bvst_node(node);
  }
}

/* Free a BVST */
void
dmnsn_delete_bvst(dmnsn_bvst *tree)
{
  if (tree) {
    dmnsn_delete_bvst_recursive(tree->root);
    free(tree);
  }
}

/* Expand node to contain the bounding box from min to max */
static void dmnsn_bvst_node_swallow(dmnsn_bvst_node *node,
                                    dmnsn_bounding_box box);

/* Insert an object into the tree */
void
dmnsn_bvst_insert(dmnsn_bvst *tree, dmnsn_object *object)
{
  dmnsn_object_precompute(object);

  dmnsn_bvst_node *node = dmnsn_new_bvst_node(), *parent = tree->root;

  node->contains     = NULL;
  node->container    = NULL;
  node->parent       = NULL;
  node->object       = object;
  node->bounding_box = object->bounding_box;

  /* Now insert the node */

  while (parent) {
    if (dmnsn_bounding_box_contains(parent->bounding_box,
                                    node->bounding_box.min)
        && dmnsn_bounding_box_contains(parent->bounding_box,
                                       node->bounding_box.max))
    {
      /* parent fully contains node */
      if (parent->contains) {
        parent = parent->contains;
      } else {
        /* We found our parent; insert node into the tree */
        parent->contains = node;
        node->parent = parent;
        break;
      }
    } else {
      /* Expand node's bounding box to fully contain parent's if it doesn't
         already */
      dmnsn_bvst_node_swallow(node, parent->bounding_box);
      /* node now fully contains parent */
      if (parent->container) {
        parent = parent->container;
      } else {
        /* We found our parent; insert node into the tree */
        parent->container = node;
        node->parent = parent;
        break;
      }
    }
  }

  dmnsn_bvst_splay(tree, node);
}

/* Expand node to contain the bounding box from min to max */
static void
dmnsn_bvst_node_swallow(dmnsn_bvst_node *node, dmnsn_bounding_box box)
{
  node->bounding_box.min = dmnsn_vector_min(node->bounding_box.min, box.min);
  node->bounding_box.max = dmnsn_vector_max(node->bounding_box.max, box.max);
}

/* Tree rotations */
static void dmnsn_bvst_rotate(dmnsn_bvst_node *node);

/* Splay a node: move it to the root via tree rotations */
void
dmnsn_bvst_splay(dmnsn_bvst *tree, dmnsn_bvst_node *node)
{
  while (node->parent) {
    if (!node->parent->parent) {
      /* Zig step - we are a child of the root node */
      dmnsn_bvst_rotate(node);
      break;
    } else if ((node == node->parent->contains
                && node->parent == node->parent->parent->contains)
               || (node == node->parent->container
                   && node->parent == node->parent->parent->container)) {
      /* Zig-zig step - we are a child on the same side as our parent */
      dmnsn_bvst_rotate(node->parent);
      dmnsn_bvst_rotate(node);
    } else {
      /* Zig-zag step - we are a child on a different side than our parent is */
      dmnsn_bvst_rotate(node);
      dmnsn_bvst_rotate(node);
    }
  }
  tree->root = node;
}

/* Rotate a tree on the edge connecting node and node->parent */
static void
dmnsn_bvst_rotate(dmnsn_bvst_node *node)
{
  dmnsn_bvst_node *P, *Q, *B;
  if (node == node->parent->contains) {
    /* We are a left child; perform a right rotation:
     *
     *     Q            P
     *    / \          / \
     *   P   C  --->  A   Q
     *  / \              / \
     * A   B            B   C
     */
    Q = node->parent;
    P = node;
    /* A = node->contains; */
    B = node->container;
    /* C = node->parent->container; */

    /* First fix up the parents */
    if (Q->parent) {
      if (Q->parent->contains == Q)
        Q->parent->contains = P;
      else
        Q->parent->container = P;
    }
    P->parent = Q->parent;
    Q->parent = P;
    if (B) B->parent = Q;

    /* Then the children */
    P->container = Q;
    Q->contains  = B;
  } else {
    /* We are a right child; perform a left rotation:
     *
     *    P                Q
     *   / \              / \
     *  A   Q    --->    P   C
     *     / \          / \
     *    B   C        A   B
     */
    P = node->parent;
    Q = node;
    /* A = node->parent->contains; */
    B = node->contains;
    /* C = node->container; */

    /* First fix up the parents */
    if (P->parent) {
      if (P->parent->contains == P)
        P->parent->contains = Q;
      else
        P->parent->container = Q;
    }
    Q->parent = P->parent;
    P->parent = Q;
    if (B) B->parent = P;

    /* Then the children */
    Q->contains  = P;
    P->container = B;
  }
}

typedef struct {
  dmnsn_bvst_node *node;
  bool intersected;
  dmnsn_intersection intersection;
} dmnsn_bvst_search_result;

static dmnsn_bvst_search_result
dmnsn_bvst_search_recursive(dmnsn_bvst_node *node, dmnsn_line ray, double t);

bool
dmnsn_bvst_search(dmnsn_bvst *tree, dmnsn_line ray,
                  dmnsn_intersection *intersection)
{
  dmnsn_bvst_search_result result
    = dmnsn_bvst_search_recursive(tree->root, ray, -1.0);

  if (result.intersected) {
    dmnsn_bvst_splay(tree, result.node);
    *intersection = result.intersection;
  }

  return result.intersected;
}

static bool dmnsn_ray_box_intersection(dmnsn_line ray, dmnsn_bounding_box box,
                                       double t);

static dmnsn_bvst_search_result
dmnsn_bvst_search_recursive(dmnsn_bvst_node *node, dmnsn_line ray, double t)
{
  dmnsn_bvst_search_result result_temp, result = {
    .node = NULL,
    .intersected = false
  };

  if (!node)
    return result;

  /* Go down the right subtree first because the closest object is more likely
     to lie in the larger bounding boxes */
  result_temp = dmnsn_bvst_search_recursive(node->container, ray, t);
  if (result_temp.node && (t < 0.0 || result_temp.intersection.t < t)) {
    result = result_temp;
    t = result.intersection.t;
  }

  if (dmnsn_bounding_box_contains(node->bounding_box, ray.x0)
      || dmnsn_ray_box_intersection(ray, node->bounding_box, t))
  {
    if (dmnsn_bounding_box_contains(node->object->bounding_box, ray.x0)
        || dmnsn_ray_box_intersection(ray, node->object->bounding_box, t))
    {
      result_temp.intersected =
        (*node->object->intersection_fn)(node->object, ray,
                                         &result_temp.intersection);

      if (result_temp.intersected
          && (t < 0.0 || result_temp.intersection.t < t)) {
        result.node = node;
        result.intersected = true;
        result.intersection = result_temp.intersection;
        t = result.intersection.t;
      }
    }

    /* Go down the left subtree */
    result_temp = dmnsn_bvst_search_recursive(node->contains, ray, t);
    if (result_temp.node && (t < 0.0 || result_temp.intersection.t < t)) {
      result = result_temp;
    }
  }

  return result;
}

static bool
dmnsn_ray_box_intersection(dmnsn_line line, dmnsn_bounding_box box, double t)
{
  double t_temp;
  dmnsn_vector p;

  if (line.n.x != 0.0) {
    /* x == box.min.x */
    t_temp = (box.min.x - line.x0.x)/line.n.x;
    p = dmnsn_line_point(line, t_temp);
    if (p.y >= box.min.y && p.y <= box.max.y
        && p.z >= box.min.z && p.z <= box.max.z
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return true;

    /* x == box.max.x */
    t_temp = (box.max.x - line.x0.x)/line.n.x;
    p = dmnsn_line_point(line, t_temp);
    if (p.y >= box.min.y && p.y <= box.max.y
        && p.z >= box.min.z && p.z <= box.max.z
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return true;
  }

  if (line.n.y != 0.0) {
    /* y == box.min.y */
    t_temp = (box.min.y - line.x0.y)/line.n.y;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= box.min.x && p.x <= box.max.x
        && p.z >= box.min.z && p.z <= box.max.z
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return true;

    /* y == box.max.y */
    t_temp = (box.max.y - line.x0.y)/line.n.y;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= box.min.x && p.x <= box.max.x
        && p.z >= box.min.z && p.z <= box.max.z
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return true;
  }

  if (line.n.z != 0.0) {
    /* z == box.min.z */
    t_temp = (box.min.z - line.x0.z)/line.n.z;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= box.min.x && p.x <= box.max.x
        && p.y >= box.min.y && p.y <= box.max.y
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return true;

    /* z == box.max.z */
    t_temp = (box.max.z - line.x0.z)/line.n.z;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= box.min.x && p.x <= box.max.x
        && p.y >= box.min.y && p.y <= box.max.y
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return true;
  }

  return false;
}
