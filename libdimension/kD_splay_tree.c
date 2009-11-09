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

static dmnsn_kD_splay_node *dmnsn_new_kD_splay_node();
static void dmnsn_delete_kD_splay_node(dmnsn_kD_splay_node *node);

/* Return an empty tree */
dmnsn_kD_splay_tree *
dmnsn_new_kD_splay_tree()
{
  dmnsn_kD_splay_tree *tree = malloc(sizeof(dmnsn_kD_splay_tree));
  if (tree) {
    tree->root = NULL;
  } else {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "kD splay tree allocation failed.");
  }
  return tree;
}

/* Recursively copy the nodes of a kD splay tree */
static dmnsn_kD_splay_node *
dmnsn_kD_splay_copy_recursive(dmnsn_kD_splay_node *root)
{
  dmnsn_kD_splay_node *node = dmnsn_new_kD_splay_node();
  *node = *root;
  if (node->contains) {
    node->contains = dmnsn_kD_splay_copy_recursive(node->contains);
    node->contains->parent = node;
  }
  if (node->container) {
    node->container = dmnsn_kD_splay_copy_recursive(node->container);
    node->container->parent = node;
  }
  return node;
}

/* Copy a kD splay tree */
dmnsn_kD_splay_tree *
dmnsn_kD_splay_copy(dmnsn_kD_splay_tree *tree)
{
  dmnsn_kD_splay_tree *copy = dmnsn_new_kD_splay_tree();
  if (tree->root)
    copy->root = dmnsn_kD_splay_copy_recursive(tree->root);
  else
    copy->root = NULL;
  return copy;
}

/* Recursively free a kD splay tree */
void
dmnsn_delete_kD_splay_tree_recursive(dmnsn_kD_splay_node *node)
{
  if (node) {
    dmnsn_delete_kD_splay_tree_recursive(node->contains);
    dmnsn_delete_kD_splay_tree_recursive(node->container);
    dmnsn_delete_kD_splay_node(node);
  }
}

/* Free a kD splay tree */
void
dmnsn_delete_kD_splay_tree(dmnsn_kD_splay_tree *tree)
{
  if (tree) {
    dmnsn_delete_kD_splay_tree_recursive(tree->root);
    free(tree);
  }
}

/* Return whether node1 contains node2 */
static int dmnsn_box_contains(dmnsn_vector p,
                              dmnsn_vector min, dmnsn_vector max);
/* Expand node to contain the bounding box from min to max */
static void dmnsn_kD_splay_node_swallow(dmnsn_kD_splay_node *node,
                                        dmnsn_vector min, dmnsn_vector max);

/* Insert an object into the tree */
void
dmnsn_kD_splay_insert(dmnsn_kD_splay_tree *tree, dmnsn_object *object)
{
  dmnsn_kD_splay_node *node = dmnsn_new_kD_splay_node(), *parent = tree->root;

  /* Store the inverse of the transformation matrix */
  object->trans_inv = dmnsn_matrix_inverse(object->trans);

  node->contains = NULL;
  node->container = NULL;
  node->parent = NULL;
  node->object = object;

  /* Calculate the new bounding box by finding the minimum coordinate of the
     transformed corners of the object's original bounding box */

  node->min = dmnsn_matrix_vector_mul(object->trans, object->min);
  node->max = node->min;

  dmnsn_vector corner;
  corner = dmnsn_vector_construct(object->min.x, object->min.y, object->max.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_node_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->min.x, object->max.y, object->min.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_node_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->min.x, object->max.y, object->max.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_node_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->max.x, object->min.y, object->min.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_node_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->max.x, object->min.y, object->max.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_node_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->max.x, object->max.y, object->min.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_node_swallow(node, corner, corner);

  corner = dmnsn_vector_construct(object->max.x, object->max.y, object->max.z);
  corner = dmnsn_matrix_vector_mul(object->trans, corner);
  dmnsn_kD_splay_node_swallow(node, corner, corner);

  /* Now insert the node */

  while (parent) {
    if (dmnsn_box_contains(node->min, parent->min, parent->max)
        && dmnsn_box_contains(node->max, parent->min, parent->max)) {
      /* parent fully contains node */
      if (parent->contains)
        parent = parent->contains;
      else {
        /* We found our parent; insert node into the tree */
        parent->contains = node;
        node->parent = parent;
        break;
      }
    } else {
      /* Expand node's bounding box to fully contain parent's if it doesn't
         already */
      dmnsn_kD_splay_node_swallow(node, parent->min, parent->max);
      /* node now fully contains parent */
      if (parent->container)
        parent = parent->container;
      else {
        /* We found our parent; insert node into the tree */
        parent->container = node;
        node->parent = parent;
        break;
      }
    }
  }

  dmnsn_kD_splay(tree, node);
}

/* Return whether p is within the axis-aligned box with corners min and max */
static int
dmnsn_box_contains(dmnsn_vector p, dmnsn_vector min, dmnsn_vector max)
{
  return (p.x >= min.x && p.y >= min.y && p.z >= min.z)
    && (p.x <= max.x && p.y <= max.y && p.z <= max.z);
}

/* Expand node to contain the bounding box from min to max */
static void
dmnsn_kD_splay_node_swallow(dmnsn_kD_splay_node *node,
                            dmnsn_vector min, dmnsn_vector max)
{
  if (node->min.x > min.x) node->min.x = min.x;
  if (node->min.y > min.y) node->min.y = min.y;
  if (node->min.z > min.z) node->min.z = min.z;

  if (node->max.x < max.x) node->max.x = max.x;
  if (node->max.y < max.y) node->max.y = max.y;
  if (node->max.z < max.z) node->max.z = max.z;
}

/* Tree rotations */
static void dmnsn_kD_splay_rotate(dmnsn_kD_splay_node *node);

/* Splay a node: move it to the root via tree rotations */
void
dmnsn_kD_splay(dmnsn_kD_splay_tree *tree, dmnsn_kD_splay_node *node)
{
  while (node->parent) {
    if (!node->parent->parent) {
      /* Zig step - we are a child of the root node */
      dmnsn_kD_splay_rotate(node);
      break;
    } else if ((node == node->parent->contains
                && node->parent == node->parent->parent->contains)
               || (node == node->parent->container
                   && node->parent == node->parent->parent->container)) {
      /* Zig-zig step - we are a child on the same side as our parent */
      dmnsn_kD_splay_rotate(node->parent);
      dmnsn_kD_splay_rotate(node);
    } else {
      /* Zig-zag step - we are a child on a different side than our parent is */
      dmnsn_kD_splay_rotate(node);
      dmnsn_kD_splay_rotate(node);
    }
  }
  tree->root = node;
}

/* Rotate a tree on the edge connecting node and node->parent */
static void
dmnsn_kD_splay_rotate(dmnsn_kD_splay_node *node)
{
  dmnsn_kD_splay_node *P, *Q, *B;
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
  dmnsn_kD_splay_node *node;
  dmnsn_intersection  *intersection;
} dmnsn_kD_splay_search_result;

static dmnsn_kD_splay_search_result
dmnsn_kD_splay_search_recursive(dmnsn_kD_splay_node *node, dmnsn_line ray,
                                double t);

dmnsn_intersection *
dmnsn_kD_splay_search(dmnsn_kD_splay_tree *tree, dmnsn_line ray)
{
  dmnsn_kD_splay_search_result result
    = dmnsn_kD_splay_search_recursive(tree->root, ray, -1.0);

  if (result.node)
    dmnsn_kD_splay(tree, result.node);

  return result.intersection;
}

static int dmnsn_ray_box_intersection(dmnsn_line ray, dmnsn_vector min,
                                      dmnsn_vector max, double t);

static dmnsn_kD_splay_search_result
dmnsn_kD_splay_search_recursive(dmnsn_kD_splay_node *node, dmnsn_line ray,
                                double t)
{
  dmnsn_line ray_trans;
  dmnsn_kD_splay_search_result result = { NULL, NULL }, result_temp;

  if (!node)
    return result;

  /* Go down the right subtree first because the closest object is more likely
     to lie in the larger bounding boxes */
  result_temp = dmnsn_kD_splay_search_recursive(node->container, ray, t);
  if (result_temp.node && (t < 0.0 || result_temp.intersection->t < t)) {
    result = result_temp;
    t = result.intersection->t;
  } else {
    dmnsn_delete_intersection(result_temp.intersection);
  }

  if (dmnsn_box_contains(ray.x0, node->min, node->max)
      || dmnsn_ray_box_intersection(ray, node->min, node->max, t))
  {
    /* Transform the ray according to the object */
    ray_trans = dmnsn_matrix_line_mul(node->object->trans_inv, ray);

    if (dmnsn_box_contains(ray_trans.x0, node->object->min, node->object->max)
        || dmnsn_ray_box_intersection(ray_trans, node->object->min,
                                      node->object->max, t))
    {
      result_temp.intersection =
        (*node->object->intersection_fn)(node->object, ray_trans);

      if (result_temp.intersection
          && (t < 0.0 || result_temp.intersection->t < t)) {
        dmnsn_delete_intersection(result.intersection);
        result.node = node;
        result.intersection = result_temp.intersection;
        t = result.intersection->t;

        /* Transform the intersection back to the observer's view */
        result.intersection->ray = ray;
        result.intersection->normal = dmnsn_vector_normalize(
          dmnsn_matrix_vector_mul(
            node->object->trans,
            result.intersection->normal
          )
        );
      } else {
        dmnsn_delete_intersection(result_temp.intersection);
      }
    }

    /* Go down the left subtree */
    result_temp = dmnsn_kD_splay_search_recursive(node->contains, ray, t);
    if (result_temp.node && (t < 0.0 || result_temp.intersection->t < t)) {
      dmnsn_delete_intersection(result.intersection);
      result = result_temp;
      t = result.intersection->t;
    } else {
      dmnsn_delete_intersection(result_temp.intersection);
    }
  }

  return result;
}

static int
dmnsn_ray_box_intersection(dmnsn_line line, dmnsn_vector min, dmnsn_vector max,
                           double t)
{
  double t_temp;
  dmnsn_vector p;

  if (line.n.x != 0.0) {
    /* x == min.x */
    t_temp = (min.x - line.x0.x)/line.n.x;
    p = dmnsn_line_point(line, t_temp);
    if (p.y >= min.y && p.y <= max.y && p.z >= min.z && p.z <= max.z
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return 1;

    /* x == max.x */
    t_temp = (max.x - line.x0.x)/line.n.x;
    p = dmnsn_line_point(line, t_temp);
    if (p.y >= min.y && p.y <= max.y && p.z >= min.z && p.z <= max.z
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return 1;
  }

  if (line.n.y != 0.0) {
    /* y == -1.0 */
    t_temp = (min.y - line.x0.y)/line.n.y;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= min.x && p.x <= max.x && p.z >= min.z && p.z <= max.z
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return 1;

    /* y == 1.0 */
    t_temp = (max.y - line.x0.y)/line.n.y;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= min.x && p.x <= max.x && p.z >= min.z && p.z <= max.z
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return 1;
  }

  if (line.n.z != 0.0) {
    /* z == -1.0 */
    t_temp = (min.z - line.x0.z)/line.n.z;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return 1;

    /* z == 1.0 */
    t_temp = (max.z - line.x0.z)/line.n.z;
    p = dmnsn_line_point(line, t_temp);
    if (p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y
        && t_temp >= 0.0 && (t < 0.0 || t_temp < t))
      return 1;
  }

  return 0;
}

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
