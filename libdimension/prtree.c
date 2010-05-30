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

typedef struct dmnsn_pseudo_prtree dmnsn_pseudo_prtree;

typedef struct dmnsn_pseudo_prleaf {
  void *children[DMNSN_PRTREE_B];
  bool is_leaf;
  dmnsn_bounding_box bounding_box;
} dmnsn_pseudo_prleaf;

typedef struct dmnsn_pseudo_prnode {
  dmnsn_pseudo_prtree *left, *right;
  dmnsn_pseudo_prleaf children[6];
} dmnsn_pseudo_prnode;

struct dmnsn_pseudo_prtree {
  bool is_leaf;
  union {
    dmnsn_pseudo_prleaf leaf;
    dmnsn_pseudo_prnode node;
  };
};

/* Expand node to contain the bounding box from min to max */
static void
dmnsn_pseudo_prleaf_swallow(dmnsn_pseudo_prleaf *leaf, dmnsn_bounding_box box)
{
  leaf->bounding_box.min = dmnsn_vector_min(leaf->bounding_box.min, box.min);
  leaf->bounding_box.max = dmnsn_vector_max(leaf->bounding_box.max, box.max);
}

/* Comparator types */
enum {
  DMNSN_XMIN,
  DMNSN_YMIN,
  DMNSN_ZMIN,
  DMNSN_XMAX,
  DMNSN_YMAX,
  DMNSN_ZMAX
};

static double
dmnsn_priority_get(dmnsn_list_iterator *i, bool is_object, int comparator)
{
  dmnsn_bounding_box box;

  if (is_object) {
    dmnsn_object *object;
    dmnsn_list_get(i, &object);
    box = object->bounding_box;
  } else {
    dmnsn_prtree_node *prnode;
    dmnsn_list_get(i, &prnode);
    box = prnode->bounding_box;
  }

  switch (comparator) {
  case DMNSN_XMIN:
    return box.min.x;
  case DMNSN_YMIN:
    return box.min.y;
  case DMNSN_ZMIN:
    return box.min.z;

  case DMNSN_XMAX:
    return -box.max.x;
  case DMNSN_YMAX:
    return -box.max.y;
  case DMNSN_ZMAX:
    return -box.max.z;

  default:
    dmnsn_assert(false, "Invalid comparator.");
    return 0.0;
  }
}

/* List sorting comparators */

static bool
dmnsn_xmin_object_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, true, DMNSN_XMIN);
  double rval = dmnsn_priority_get(r, true, DMNSN_XMIN);
  return lval < rval;
}

static bool
dmnsn_xmin_prnode_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, false, DMNSN_XMIN);
  double rval = dmnsn_priority_get(r, false, DMNSN_XMIN);
  return lval < rval;
}

static bool
dmnsn_ymin_object_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, true, DMNSN_YMIN);
  double rval = dmnsn_priority_get(r, true, DMNSN_YMIN);
  return lval < rval;
}

static bool
dmnsn_ymin_prnode_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, false, DMNSN_YMIN);
  double rval = dmnsn_priority_get(r, false, DMNSN_YMIN);
  return lval < rval;
}

static bool
dmnsn_zmin_object_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, true, DMNSN_ZMIN);
  double rval = dmnsn_priority_get(r, true, DMNSN_ZMIN);
  return lval < rval;
}

static bool
dmnsn_zmin_prnode_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, false, DMNSN_ZMIN);
  double rval = dmnsn_priority_get(r, false, DMNSN_ZMIN);
  return lval < rval;
}

static bool
dmnsn_xmax_object_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, true, DMNSN_XMAX);
  double rval = dmnsn_priority_get(r, true, DMNSN_XMAX);
  return lval < rval;
}

static bool
dmnsn_xmax_prnode_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, false, DMNSN_XMAX);
  double rval = dmnsn_priority_get(r, false, DMNSN_XMAX);
  return lval < rval;
}

static bool
dmnsn_ymax_object_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, true, DMNSN_YMAX);
  double rval = dmnsn_priority_get(r, true, DMNSN_YMAX);
  return lval < rval;
}

static bool
dmnsn_ymax_prnode_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, false, DMNSN_YMAX);
  double rval = dmnsn_priority_get(r, false, DMNSN_YMAX);
  return lval < rval;
}

static bool
dmnsn_zmax_object_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, true, DMNSN_ZMAX);
  double rval = dmnsn_priority_get(r, true, DMNSN_ZMAX);
  return lval < rval;
}

static bool
dmnsn_zmax_prnode_comp(dmnsn_list_iterator *l, dmnsn_list_iterator *r)
{
  double lval = dmnsn_priority_get(l, false, DMNSN_ZMAX);
  double rval = dmnsn_priority_get(r, false, DMNSN_ZMAX);
  return lval < rval;
}

static dmnsn_list_comparator_fn *dmnsn_object_comparators[6] = {
  [DMNSN_XMIN] = &dmnsn_xmin_object_comp,
  [DMNSN_YMIN] = &dmnsn_ymin_object_comp,
  [DMNSN_ZMIN] = &dmnsn_zmin_object_comp,
  [DMNSN_XMAX] = &dmnsn_xmax_object_comp,
  [DMNSN_YMAX] = &dmnsn_ymax_object_comp,
  [DMNSN_ZMAX] = &dmnsn_zmax_object_comp
};

static dmnsn_list_comparator_fn *dmnsn_prnode_comparators[6] = {
  [DMNSN_XMIN] = &dmnsn_xmin_prnode_comp,
  [DMNSN_YMIN] = &dmnsn_ymin_prnode_comp,
  [DMNSN_ZMIN] = &dmnsn_zmin_prnode_comp,
  [DMNSN_XMAX] = &dmnsn_xmax_prnode_comp,
  [DMNSN_YMAX] = &dmnsn_ymax_prnode_comp,
  [DMNSN_ZMAX] = &dmnsn_zmax_prnode_comp
};

static dmnsn_list_iterator *
dmnsn_priority_search(dmnsn_list *leaves, bool are_objects, int comparator)
{
  dmnsn_list_iterator *i = dmnsn_list_first(leaves);
  if (i) {
    double candidate = dmnsn_priority_get(i, are_objects, comparator);

    for (dmnsn_list_iterator *j = dmnsn_list_next(i);
         j != NULL;
         j = dmnsn_list_next(j))
    {
      double new_candidate = dmnsn_priority_get(j, are_objects, comparator);
      if (new_candidate < candidate) {
        candidate = new_candidate;
        i = j;
      }
    }
  }

  return i;
}

/* Build a pseudo PR-tree */
static dmnsn_pseudo_prtree *
dmnsn_new_pseudo_prtree(dmnsn_list *leaves, bool are_objects, int comparator)
{
  dmnsn_pseudo_prtree *pseudo = dmnsn_malloc(sizeof(dmnsn_pseudo_prtree));

  if (dmnsn_list_size(leaves) <= DMNSN_PRTREE_B) {
    /* Make a leaf */
    pseudo->is_leaf = true;
    pseudo->leaf.bounding_box.min = dmnsn_zero;
    pseudo->leaf.bounding_box.max = dmnsn_zero;

    size_t i;
    dmnsn_list_iterator *ii;
    if (are_objects) {
      pseudo->leaf.is_leaf = true;
      for (i = 0, ii = dmnsn_list_first(leaves);
           ii != NULL;
           ++i, ii = dmnsn_list_next(ii))
      {
        dmnsn_object *object;
        dmnsn_list_get(ii, &object);

        pseudo->leaf.children[i] = object;
        if (i == 0) {
          pseudo->leaf.bounding_box = object->bounding_box;
        } else {
          dmnsn_pseudo_prleaf_swallow(&pseudo->leaf, object->bounding_box);
        }
      }
    } else {
      pseudo->leaf.is_leaf = false;
      for (i = 0, ii = dmnsn_list_first(leaves);
           ii != NULL;
           ++i, ii = dmnsn_list_next(ii))
      {
        dmnsn_prtree_node *prnode;
        dmnsn_list_get(ii, &prnode);

        pseudo->leaf.children[i] = prnode;
        if (i == 0) {
          pseudo->leaf.bounding_box = prnode->bounding_box;
        } else {
          dmnsn_pseudo_prleaf_swallow(&pseudo->leaf, prnode->bounding_box);
        }
      }
    }

    for (; i < DMNSN_PRTREE_B; ++i) {
      pseudo->leaf.children[i] = NULL;
    }
  } else {
    /* Make an internal node */
    pseudo->is_leaf = false;
    for (size_t i = 0; i < 6; ++i) {
      pseudo->node.children[i].is_leaf = are_objects;
    }

    /* Fill the priority leaves */
    size_t i, j;
    for (i = 0; i < DMNSN_PRTREE_B; ++i) {
      for (j = 0; j < 6; ++j) {
        dmnsn_list_iterator *k = dmnsn_priority_search(leaves, are_objects, j);
        if (!k)
          break;

        if (are_objects) {
          dmnsn_object *object;
          dmnsn_list_get(k, &object);
          pseudo->node.children[j].children[i] = object;
          if (i == 0) {
            pseudo->node.children[j].bounding_box = object->bounding_box;
          } else {
            dmnsn_pseudo_prleaf_swallow(&pseudo->node.children[j],
                                        object->bounding_box);
          }
        } else {
          dmnsn_prtree_node *prnode;
          dmnsn_list_get(k, &prnode);
          pseudo->node.children[j].children[i] = prnode;
          if (i == 0) {
            pseudo->node.children[j].bounding_box = prnode->bounding_box;
          } else {
            dmnsn_pseudo_prleaf_swallow(&pseudo->node.children[j],
                                        prnode->bounding_box);
          }
        }

        dmnsn_list_remove(leaves, k);
      }

      if (dmnsn_list_size(leaves) == 0)
        break;
    }

    /* Set remaining space in the priority leaves to NULL */
    for (; i < DMNSN_PRTREE_B; ++i) {
      for (; j < 6; ++j) {
        if (i == 0) {
          pseudo->node.children[j].bounding_box.min = dmnsn_zero;
          pseudo->node.children[j].bounding_box.max = dmnsn_zero;
        }
        pseudo->node.children[j].children[i] = NULL;
      }
      j = 0;
    }

    /* Recursively build the subtrees */
    if (are_objects)
      dmnsn_list_sort(leaves, dmnsn_object_comparators[comparator]);
    else
      dmnsn_list_sort(leaves, dmnsn_prnode_comparators[comparator]);

    dmnsn_list *half = dmnsn_list_split(leaves);
    pseudo->node.left
      = dmnsn_new_pseudo_prtree(leaves, are_objects, (comparator + 1)%6);
    pseudo->node.right
      = dmnsn_new_pseudo_prtree(half, are_objects, (comparator + 1)%6);
    dmnsn_delete_list(half);
  }

  return pseudo;
}

static void
dmnsn_delete_pseudo_prtree(dmnsn_pseudo_prtree *pseudo)
{
  if (pseudo) {
    if (!pseudo->is_leaf) {
      dmnsn_delete_pseudo_prtree(pseudo->node.left);
      dmnsn_delete_pseudo_prtree(pseudo->node.right);
    }
    free(pseudo);
  }
}

/* Construct a node from a pseudo leaf */
static dmnsn_prtree_node *
dmnsn_new_prtree_node(const dmnsn_pseudo_prleaf *leaf)
{
  dmnsn_prtree_node *node = dmnsn_malloc(sizeof(dmnsn_prtree_node));
  node->is_leaf      = leaf->is_leaf;
  node->bounding_box = leaf->bounding_box;

  for (size_t i = 0; i < DMNSN_PRTREE_B; ++i) {
    if (!leaf->children[i]) {
      node->children[i] = NULL;
    } else if (leaf->is_leaf) {
      dmnsn_object *object = leaf->children[i];
      node->children[i] = object;
      node->bounding_boxes[i] = object->bounding_box;
    } else {
      dmnsn_prtree_node *child = leaf->children[i];
      node->children[i] = child;
      node->bounding_boxes[i] = child->bounding_box;
    }
  }

  return node;
}

static void
dmnsn_pseudo_prtree_add_leaf(const dmnsn_pseudo_prleaf *leaf,
                             dmnsn_list *leaves)
{
  /* Don't add empty leaves */
  if (leaf->children[0]) {
    dmnsn_prtree_node *prnode = dmnsn_new_prtree_node(leaf);
    dmnsn_list_push(leaves, &prnode);
  }
}

static void
dmnsn_pseudo_prtree_leaves_recursive(const dmnsn_pseudo_prtree *node,
                                     dmnsn_list *leaves)
{
  if (node->is_leaf) {
    dmnsn_pseudo_prtree_add_leaf(&node->leaf, leaves);
  } else {
    for (size_t i = 0; i < 6; ++i) {
      dmnsn_pseudo_prtree_add_leaf(&node->node.children[i], leaves);
    }
    dmnsn_pseudo_prtree_leaves_recursive(node->node.left, leaves);
    dmnsn_pseudo_prtree_leaves_recursive(node->node.right, leaves);
  }
}

/* Extract the leaves of a pseudo PR-tree */
static dmnsn_list *
dmnsn_pseudo_prtree_leaves(const dmnsn_pseudo_prtree *pseudo)
{
  dmnsn_list *leaves = dmnsn_new_list(sizeof(dmnsn_prtree_node *));
  dmnsn_pseudo_prtree_leaves_recursive(pseudo, leaves);

  if (dmnsn_list_size(leaves) == 0) {
    dmnsn_prtree_node *prnode = dmnsn_new_prtree_node(&pseudo->leaf);
    dmnsn_list_push(leaves, &prnode);
  }

  return leaves;
}

/* Split the unbounded objects into a new list */
static dmnsn_list *
dmnsn_split_unbounded(dmnsn_list *objects)
{
  dmnsn_list *unbounded = dmnsn_new_list(sizeof(dmnsn_object *));

  dmnsn_list_iterator *i = dmnsn_list_first(objects);
  while (i) {
    dmnsn_object *object;
    dmnsn_list_get(i, &object);

    if (isinf(object->bounding_box.min.x)) {
      dmnsn_list_iterator *next = dmnsn_list_next(i);
      dmnsn_list_iterator_remove(objects, i);
      dmnsn_list_iterator_insert(unbounded, NULL, i);
      i = next;
    } else {
      i = dmnsn_list_next(i);
    }
  }

  return unbounded;
}

/* Construct a PR-tree from a bulk of objects */
dmnsn_prtree *
dmnsn_new_prtree(const dmnsn_array *objects)
{
  dmnsn_list *leaves = dmnsn_list_from_array(objects);
  dmnsn_list *unbounded = dmnsn_split_unbounded(leaves);

  dmnsn_pseudo_prtree *pseudo = dmnsn_new_pseudo_prtree(leaves, true, 0);
  dmnsn_delete_list(leaves);
  leaves = dmnsn_pseudo_prtree_leaves(pseudo);
  dmnsn_delete_pseudo_prtree(pseudo);

  while (dmnsn_list_size(leaves) > 1) {
    pseudo = dmnsn_new_pseudo_prtree(leaves, false, 0);
    dmnsn_delete_list(leaves);
    leaves = dmnsn_pseudo_prtree_leaves(pseudo);
    dmnsn_delete_pseudo_prtree(pseudo);
  }

  dmnsn_prtree *prtree = dmnsn_malloc(sizeof(dmnsn_prtree));
  dmnsn_list_get(dmnsn_list_first(leaves), &prtree->root);
  prtree->unbounded = dmnsn_array_from_list(unbounded);

  dmnsn_delete_list(unbounded);
  dmnsn_delete_list(leaves);
  return prtree;
}

/* Free a PR-tree node */
void
dmnsn_delete_prtree_node(dmnsn_prtree_node *node)
{
  if (node) {
    if (!node->is_leaf) {
      for (size_t i = 0; i < DMNSN_PRTREE_B; ++i) {
        dmnsn_delete_prtree_node(node->children[i]);
      }
    }
    free(node);
  }
}

/* Free a PR-tree */
void
dmnsn_delete_prtree(dmnsn_prtree *tree)
{
  if (tree) {
    dmnsn_delete_prtree_node(tree->root);
    dmnsn_delete_array(tree->unbounded);
    free(tree);
  }
}

/* Ray-AABB intersection test */
static inline bool
dmnsn_ray_box_intersection(dmnsn_line line, dmnsn_bounding_box box, double t)
{
  double tmin = -INFINITY, tmax = INFINITY;

  if (line.n.x != 0.0) {
    double tx1 = (box.min.x - line.x0.x)/line.n.x;
    double tx2 = (box.max.x - line.x0.x)/line.n.x;

    tmin = dmnsn_max(tmin, dmnsn_min(tx1, tx2));
    tmax = dmnsn_min(tmax, dmnsn_max(tx1, tx2));

    if (tmin > tmax)
      return false;
  } else {
    if (line.x0.x < box.min.x || line.x0.x > box.max.x)
      return false;
  }

  if (line.n.y != 0.0) {
    double ty1 = (box.min.y - line.x0.y)/line.n.y;
    double ty2 = (box.max.y - line.x0.y)/line.n.y;

    tmin = dmnsn_max(tmin, dmnsn_min(ty1, ty2));
    tmax = dmnsn_min(tmax, dmnsn_max(ty1, ty2));

    if (tmin > tmax)
      return false;
  } else {
    if (line.x0.y < box.min.y || line.x0.y > box.max.y)
      return false;
  }

  if (line.n.z != 0.0) {
    double tz1 = (box.min.z - line.x0.z)/line.n.z;
    double tz2 = (box.max.z - line.x0.z)/line.n.z;

    tmin = dmnsn_max(tmin, dmnsn_min(tz1, tz2));
    tmax = dmnsn_min(tmax, dmnsn_max(tz1, tz2));

    if (tmin > tmax)
      return false;
  } else {
    if (line.x0.z < box.min.z || line.x0.z > box.max.z)
      return false;
  }

  if (tmax < 0.0)
    return false;

  return tmin < t;
}

static void
dmnsn_prtree_search_recursive(const dmnsn_prtree_node *node, dmnsn_line ray,
                              dmnsn_intersection *intersection, double *t)
{
  for (size_t i = 0; i < DMNSN_PRTREE_B; ++i) {
    if (!node->children[i])
      break;

    if (dmnsn_ray_box_intersection(ray, node->bounding_boxes[i], *t)) {
      if (node->is_leaf) {
        dmnsn_object *object = node->children[i];

        dmnsn_intersection local_intersection;
        if ((*object->intersection_fn)(object, ray, &local_intersection)) {
          if (local_intersection.t < *t) {
            *intersection = local_intersection;
            *t = local_intersection.t;
          }
        }
      } else {
        dmnsn_prtree_search_recursive(node->children[i], ray, intersection, t);
      }
    }
  }
}

bool
dmnsn_prtree_search(const dmnsn_prtree *tree, dmnsn_line ray,
                    dmnsn_intersection *intersection)
{
  double t = INFINITY;

  /* Search the unbounded objects */
  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, tree->unbounded) {
    dmnsn_intersection local_intersection;
    if ((*(*object)->intersection_fn)(*object, ray, &local_intersection)) {
      if (local_intersection.t < t) {
        *intersection = local_intersection;
        t = local_intersection.t;
      }
    }
  }

  /* Search the bounded objects */
  if (dmnsn_ray_box_intersection(ray, tree->root->bounding_box, t)) {
    dmnsn_prtree_search_recursive(tree->root, ray, intersection, &t);
  }

  return !isinf(t);
}
