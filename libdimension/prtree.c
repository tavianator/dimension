/*************************************************************************
 * Copyright (C) 2010-2012 Tavian Barnes <tavianator@tavianator.com>     *
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

/**
 * @file
 * Priority R-tree implementation.
 */

#include "dimension-internal.h"
#include <stdlib.h>

/** Number of children per PR-node. */
#define DMNSN_PRTREE_B 8
/** Number of priority leaves per pseudo-PR-node (must be 2*ndimensions). */
#define DMNSN_PSEUDO_B 6

/** The side of the split that a node ended up on. */
typedef enum dmnsn_prnode_location {
  DMNSN_PRTREE_LEAF, /**< Priority leaf. */
  DMNSN_PRTREE_LEFT, /**< Left child. */
  DMNSN_PRTREE_RIGHT /**< Right child. */
} dmnsn_prnode_location;

/** Construct an empty PR-node. */
static inline dmnsn_bvh_node *
dmnsn_new_prnode(void)
{
  dmnsn_bvh_node *node = dmnsn_new_bvh_node(DMNSN_PRTREE_B);
  node->data = DMNSN_PRTREE_LEFT; /* Mustn't be _LEAF */
  return node;
}

/** Comparator types. */
enum {
  DMNSN_XMIN,
  DMNSN_YMIN,
  DMNSN_ZMIN,
  DMNSN_XMAX,
  DMNSN_YMAX,
  DMNSN_ZMAX
};

/** Get a coordinate of the bounding box of a node. */
static inline double
dmnsn_get_coordinate(const dmnsn_bvh_node * const *node, int comparator)
{
  switch (comparator) {
  case DMNSN_XMIN:
    return (*node)->bounding_box.min.x;
  case DMNSN_YMIN:
    return (*node)->bounding_box.min.y;
  case DMNSN_ZMIN:
    return (*node)->bounding_box.min.z;

  case DMNSN_XMAX:
    return -(*node)->bounding_box.max.x;
  case DMNSN_YMAX:
    return -(*node)->bounding_box.max.y;
  case DMNSN_ZMAX:
    return -(*node)->bounding_box.max.z;

  default:
    dmnsn_unreachable("Invalid comparator.");
  }
}

/* List sorting comparators */

static int
dmnsn_xmin_comp(const void *l, const void *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_XMIN);
  double rval = dmnsn_get_coordinate(r, DMNSN_XMIN);
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_ymin_comp(const void *l, const void *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_YMIN);
  double rval = dmnsn_get_coordinate(r, DMNSN_YMIN);
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_zmin_comp(const void *l, const void *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_ZMIN);
  double rval = dmnsn_get_coordinate(r, DMNSN_ZMIN);
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_xmax_comp(const void *l, const void *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_XMAX);
  double rval = dmnsn_get_coordinate(r, DMNSN_XMAX);
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_ymax_comp(const void *l, const void *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_YMAX);
  double rval = dmnsn_get_coordinate(r, DMNSN_YMAX);
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_zmax_comp(const void *l, const void *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_ZMAX);
  double rval = dmnsn_get_coordinate(r, DMNSN_ZMAX);
  return (lval > rval) - (lval < rval);
}

/** All comparators. */
static dmnsn_array_comparator_fn *const dmnsn_comparators[DMNSN_PSEUDO_B] = {
  [DMNSN_XMIN] = dmnsn_xmin_comp,
  [DMNSN_YMIN] = dmnsn_ymin_comp,
  [DMNSN_ZMIN] = dmnsn_zmin_comp,
  [DMNSN_XMAX] = dmnsn_xmax_comp,
  [DMNSN_YMAX] = dmnsn_ymax_comp,
  [DMNSN_ZMAX] = dmnsn_zmax_comp
};

/** Add the priority leaves for this level. */
static void
dmnsn_add_priority_leaves(dmnsn_array *sorted_leaves[DMNSN_PSEUDO_B],
                          dmnsn_array *new_leaves)
{
  for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
    dmnsn_bvh_node *leaf = NULL;
    dmnsn_bvh_node **leaves = dmnsn_array_first(sorted_leaves[i]);
    for (size_t j = 0, size = dmnsn_array_size(sorted_leaves[i]);
         j < size && (!leaf || leaf->nchildren < DMNSN_PRTREE_B);
         ++j)
    {
      /* Skip all the previously found extreme nodes */
      if (leaves[j]->data == DMNSN_PRTREE_LEAF) {
        continue;
      }

      if (!leaf) {
        leaf = dmnsn_new_prnode();
      }
      leaves[j]->data = DMNSN_PRTREE_LEAF;
      dmnsn_bvh_node_add(leaf, leaves[j]);
    }

    if (leaf) {
      dmnsn_array_push(new_leaves, &leaf);
    } else {
      return;
    }
  }
}

/** Split the sorted lists into the left and right subtrees. */
static bool
dmnsn_split_sorted_leaves(dmnsn_array *sorted_leaves[DMNSN_PSEUDO_B],
                          dmnsn_array *right_sorted_leaves[DMNSN_PSEUDO_B],
                          size_t i)
{
  /* Get rid of the extreme nodes */
  dmnsn_bvh_node **leaves = dmnsn_array_first(sorted_leaves[i]);
  size_t j, skip;
  for (j = 0, skip = 0; j < dmnsn_array_size(sorted_leaves[i]); ++j) {
    if (leaves[j]->data == DMNSN_PRTREE_LEAF) {
      ++skip;
    } else {
      leaves[j - skip] = leaves[j];
    }
  }
  dmnsn_array_resize(sorted_leaves[i], j - skip);

  if (dmnsn_array_size(sorted_leaves[i]) == 0) {
    return false;
  }

  /* Split the appropriate list and mark the left and right child nodes */
  right_sorted_leaves[i] = dmnsn_array_split(sorted_leaves[i]);
  DMNSN_ARRAY_FOREACH (dmnsn_bvh_node **, node, sorted_leaves[i]) {
    (*node)->data = DMNSN_PRTREE_LEFT;
  }
  DMNSN_ARRAY_FOREACH (dmnsn_bvh_node **, node, right_sorted_leaves[i]) {
    (*node)->data = DMNSN_PRTREE_RIGHT;
  }

  /* Split the rest of the lists */
  for (size_t j = 0; j < DMNSN_PSEUDO_B; ++j) {
    if (j != i) {
      right_sorted_leaves[j] = dmnsn_new_array(sizeof(dmnsn_bvh_node *));

      dmnsn_bvh_node **leaves = dmnsn_array_first(sorted_leaves[j]);
      size_t k, skip;
      for (k = 0, skip = 0; k < dmnsn_array_size(sorted_leaves[j]); ++k) {
        if (leaves[k]->data == DMNSN_PRTREE_LEAF) {
          ++skip;
        } else if (leaves[k]->data == DMNSN_PRTREE_RIGHT) {
          dmnsn_array_push(right_sorted_leaves[j], &leaves[k]);
          ++skip;
        } else {
          leaves[k - skip] = leaves[k];
        }
      }
      dmnsn_array_resize(sorted_leaves[j], k - skip);
    }
  }

  return true;
}

/** Recursively constructs an implicit pseudo-PR-tree and collects the priority
    leaves. */
static void
dmnsn_priority_leaves_recursive(dmnsn_array *sorted_leaves[DMNSN_PSEUDO_B],
                                dmnsn_array *new_leaves,
                                int comparator)
{
  dmnsn_add_priority_leaves(sorted_leaves, new_leaves);

  dmnsn_array *right_sorted_leaves[DMNSN_PSEUDO_B];
  if (dmnsn_split_sorted_leaves(sorted_leaves, right_sorted_leaves, comparator))
  {
    dmnsn_priority_leaves_recursive(right_sorted_leaves, new_leaves,
                                    (comparator + 1)%DMNSN_PSEUDO_B);
    for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
      dmnsn_delete_array(right_sorted_leaves[i]);
    }

    dmnsn_priority_leaves_recursive(sorted_leaves, new_leaves,
                                    (comparator + 1)%DMNSN_PSEUDO_B);
  }
}

/** Constructs an implicit pseudo-PR-tree and returns the priority leaves. */
static dmnsn_array *
dmnsn_priority_leaves(const dmnsn_array *leaves)
{
  dmnsn_array *sorted_leaves[DMNSN_PSEUDO_B];
  for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
    sorted_leaves[i] = dmnsn_array_copy(leaves);
    dmnsn_array_sort(sorted_leaves[i], dmnsn_comparators[i]);
  }

  dmnsn_array *new_leaves = dmnsn_new_array(sizeof(dmnsn_bvh_node *));
  dmnsn_priority_leaves_recursive(sorted_leaves, new_leaves, 0);

  for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
    dmnsn_delete_array(sorted_leaves[i]);
  }

  return new_leaves;
}

dmnsn_bvh_node *
dmnsn_new_prtree(const dmnsn_array *objects)
{
  if (dmnsn_array_size(objects) == 0) {
    return NULL;
  }

  /* Make the initial array of leaves */
  dmnsn_array *leaves = dmnsn_new_array(sizeof(dmnsn_bvh_node *));
  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, objects) {
    dmnsn_bvh_node *node = dmnsn_new_bvh_leaf_node(*object);
    node->data = DMNSN_PRTREE_LEFT; /* Mustn't be _LEAF */
    dmnsn_array_push(leaves, &node);
  }

  while (dmnsn_array_size(leaves) > 1) {
    dmnsn_array *new_leaves = dmnsn_priority_leaves(leaves);
    dmnsn_delete_array(leaves);
    leaves = new_leaves;
  }

  dmnsn_bvh_node *root = *(dmnsn_bvh_node **)dmnsn_array_first(leaves);
  dmnsn_delete_array(leaves);
  return root;
}
