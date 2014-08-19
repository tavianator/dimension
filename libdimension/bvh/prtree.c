/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "internal/platform.h"
#include "internal/prtree.h"
#include "internal/concurrency.h"
#include <stdlib.h>

/// Number of children per PR-node.
#define DMNSN_PRTREE_B 8
/// Number of priority leaves per pseudo-PR-node (must be 2*ndimensions).
#define DMNSN_PSEUDO_B 6

/// The side of the split that a node ended up on.
typedef enum dmnsn_prnode_color {
  DMNSN_PRTREE_LEAF, ///< Priority leaf.
  DMNSN_PRTREE_LEFT, ///< Left child.
  DMNSN_PRTREE_RIGHT ///< Right child.
} dmnsn_prnode_color;

/**
 * A BVH node with associated color.  Compared to storing the color in the
 * \p dmnsn_bvh_node itself, this method has decreased cache performance during
 * sorting (due to an extra pointer chase), but increased performance during
 * tree building (because it's much smaller than a full \p dmnsn_bvh_node).
 * Overall it gives about a 25% improvement.
 */
typedef struct dmnsn_colored_prnode {
  dmnsn_prnode_color color;
  dmnsn_bvh_node *node;
} dmnsn_colored_prnode;

/// Construct an empty PR-node.
static inline dmnsn_bvh_node *
dmnsn_new_prnode(void)
{
  return dmnsn_new_bvh_node(DMNSN_PRTREE_B);
}

/// Comparator types.
enum {
  DMNSN_XMIN,
  DMNSN_YMIN,
  DMNSN_ZMIN,
  DMNSN_XMAX,
  DMNSN_YMAX,
  DMNSN_ZMAX
};

// List sorting comparators

static int
dmnsn_xmin_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_colored_prnode **)l)->node->aabb.min.x;
  double rval = (*(const dmnsn_colored_prnode **)r)->node->aabb.min.x;
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_ymin_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_colored_prnode **)l)->node->aabb.min.y;
  double rval = (*(const dmnsn_colored_prnode **)r)->node->aabb.min.y;
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_zmin_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_colored_prnode **)l)->node->aabb.min.z;
  double rval = (*(const dmnsn_colored_prnode **)r)->node->aabb.min.z;
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_xmax_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_colored_prnode **)l)->node->aabb.max.x;
  double rval = (*(const dmnsn_colored_prnode **)r)->node->aabb.max.x;
  return (lval < rval) - (lval > rval);
}

static int
dmnsn_ymax_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_colored_prnode **)l)->node->aabb.max.y;
  double rval = (*(const dmnsn_colored_prnode **)r)->node->aabb.max.y;
  return (lval < rval) - (lval > rval);
}

static int
dmnsn_zmax_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_colored_prnode **)l)->node->aabb.max.z;
  double rval = (*(const dmnsn_colored_prnode **)r)->node->aabb.max.z;
  return (lval < rval) - (lval > rval);
}

/// All comparators.
static dmnsn_array_comparator_fn *const dmnsn_comparators[DMNSN_PSEUDO_B] = {
  [DMNSN_XMIN] = dmnsn_xmin_comp,
  [DMNSN_YMIN] = dmnsn_ymin_comp,
  [DMNSN_ZMIN] = dmnsn_zmin_comp,
  [DMNSN_XMAX] = dmnsn_xmax_comp,
  [DMNSN_YMAX] = dmnsn_ymax_comp,
  [DMNSN_ZMAX] = dmnsn_zmax_comp,
};

/// Add the priority leaves for this level.
static void
dmnsn_add_priority_leaves(dmnsn_colored_prnode **sorted_leaves[DMNSN_PSEUDO_B],
                          size_t start, size_t end,
                          dmnsn_array *new_leaves)
{
  for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
    dmnsn_bvh_node *leaf = NULL;
    dmnsn_colored_prnode **leaves = sorted_leaves[i];

    for (size_t j = start;
         j < end && (!leaf || leaf->nchildren < DMNSN_PRTREE_B);
         ++j) {
      // Skip all the previously found extreme nodes
      if (leaves[j]->color == DMNSN_PRTREE_LEAF) {
        continue;
      }

      if (!leaf) {
        leaf = dmnsn_new_prnode();
      }
      leaves[j]->color = DMNSN_PRTREE_LEAF;
      dmnsn_bvh_node_add(leaf, leaves[j]->node);
    }

    if (leaf) {
      dmnsn_array_push(new_leaves, &leaf);
    } else {
      return;
    }
  }
}

/// Get rid of the extreme nodes.
static void
dmnsn_filter_priority_leaves(dmnsn_colored_prnode **leaves, size_t start, size_t *endp)
{
  size_t i, skip, end;
  for (i = start, skip = 0, end = *endp; i < end; ++i) {
    if (leaves[i]->color == DMNSN_PRTREE_LEAF) {
      ++skip;
    } else {
      leaves[i - skip] = leaves[i];
    }
  }

  *endp -= skip;
}

/// Split the leaves and mark the left and right child nodes.
static void
dmnsn_split_sorted_leaves_easy(dmnsn_colored_prnode **leaves, size_t start, size_t *midp, size_t end)
{
  size_t i, mid = start + (end - start + 1)/2;
  for (i = start; i < mid; ++i) {
    leaves[i]->color = DMNSN_PRTREE_LEFT;
  }
  for (; i < end; ++i) {
    leaves[i]->color = DMNSN_PRTREE_RIGHT;
  }

  *midp = mid;
}

/// Split the leaves using the coloring from dmnsn_split_sorted_leaves_easy().
static void
dmnsn_split_sorted_leaves_hard(dmnsn_colored_prnode **leaves, dmnsn_colored_prnode **buffer, size_t start, size_t end)
{
  size_t i, j, skip;
  for (i = start, j = 0, skip = 0; i < end; ++i) {
    if (leaves[i]->color == DMNSN_PRTREE_LEFT) {
      leaves[i - skip] = leaves[i];
    } else {
      if (leaves[i]->color == DMNSN_PRTREE_RIGHT) {
        buffer[j] = leaves[i];
        ++j;
      }
      ++skip;
    }
  }

  size_t mid = i - skip;
  for (i = 0; i < j; ++i) {
    leaves[mid + i] = buffer[i];
  }
}

/// Split the sorted lists into the left and right subtrees.
static void
dmnsn_split_sorted_leaves(dmnsn_colored_prnode **sorted_leaves[DMNSN_PSEUDO_B],
                          size_t start, size_t *midp, size_t *endp,
                          dmnsn_colored_prnode **buffer, int i)
{
  size_t orig_end = *endp;

  // Filter the extreme nodes in the ith list
  dmnsn_filter_priority_leaves(sorted_leaves[i], start, endp);

  // Split the ith list
  dmnsn_split_sorted_leaves_easy(sorted_leaves[i], start, midp, *endp);

  // Split the rest of the lists
  for (size_t j = 0; j < DMNSN_PSEUDO_B; ++j) {
    if (j == i) {
      continue;
    }

    dmnsn_split_sorted_leaves_hard(sorted_leaves[j], buffer, start, orig_end);
  }
}

/// Recursively constructs an implicit pseudo-PR-tree and collects the priority
/// leaves.
static void
dmnsn_priority_leaves_recursive(dmnsn_colored_prnode **sorted_leaves[DMNSN_PSEUDO_B],
                                size_t start, size_t end,
                                dmnsn_colored_prnode **buffer,
                                dmnsn_array *new_leaves,
                                int comparator)
{
  dmnsn_add_priority_leaves(sorted_leaves, start, end, new_leaves);

  size_t mid;
  dmnsn_split_sorted_leaves(sorted_leaves, start, &mid, &end, buffer, comparator);

  int next = (comparator + 1)%DMNSN_PSEUDO_B;

  if (start < mid) {
    dmnsn_priority_leaves_recursive(sorted_leaves, start, mid, buffer, new_leaves, next);
  }

  if (mid < end) {
    dmnsn_priority_leaves_recursive(sorted_leaves, mid, end, buffer, new_leaves, next);
  }
}

/// Sort each dimension in parallel with more than this many leaves.
#define DMNSN_PARALLEL_SORT_THRESHOLD 1024

typedef struct {
  dmnsn_colored_prnode *colored_leaves;
  dmnsn_colored_prnode ***sorted_leaves;
  size_t nleaves;
} dmnsn_sort_leaves_payload;

static dmnsn_colored_prnode **
dmnsn_sort_leaf_array(dmnsn_colored_prnode *colored_leaves, size_t nleaves, int comparator)
{
  dmnsn_colored_prnode **sorted_leaves = dmnsn_malloc(nleaves*sizeof(dmnsn_colored_prnode *));

  for (size_t i = 0; i < nleaves; ++i) {
    sorted_leaves[i] = colored_leaves + i;
  }

  qsort(sorted_leaves, nleaves, sizeof(dmnsn_colored_prnode *), dmnsn_comparators[comparator]);

  return sorted_leaves;
}

static int
dmnsn_sort_leaves(void *ptr, unsigned int thread, unsigned int nthreads)
{
  dmnsn_sort_leaves_payload *payload = ptr;

  for (unsigned int i = thread; i < DMNSN_PSEUDO_B; i += nthreads) {
    payload->sorted_leaves[i] = dmnsn_sort_leaf_array(payload->colored_leaves, payload->nleaves, i);
  }

  return 0;
}

/// Constructs an implicit pseudo-PR-tree and returns the priority leaves.
static dmnsn_array *
dmnsn_priority_leaves(const dmnsn_array *leaves, unsigned int nthreads)
{
  dmnsn_bvh_node **leaves_arr = dmnsn_array_first(leaves);
  size_t nleaves = dmnsn_array_size(leaves);

  dmnsn_colored_prnode *colored_leaves = dmnsn_malloc(nleaves*sizeof(dmnsn_colored_prnode));
  for (size_t i = 0; i < nleaves; ++i) {
    colored_leaves[i].color = DMNSN_PRTREE_LEFT; // Mustn't be _LEAF
    colored_leaves[i].node = leaves_arr[i];
  }

  dmnsn_colored_prnode **sorted_leaves[DMNSN_PSEUDO_B];

  if (nleaves >= DMNSN_PARALLEL_SORT_THRESHOLD && nthreads > 1) {
    dmnsn_sort_leaves_payload payload = {
      .colored_leaves = colored_leaves,
      .sorted_leaves = sorted_leaves,
      .nleaves = nleaves,
    };
    dmnsn_execute_concurrently(NULL, dmnsn_sort_leaves, &payload, nthreads);
  } else {
    for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
      sorted_leaves[i] = dmnsn_sort_leaf_array(colored_leaves, nleaves, i);
    }
  }

  size_t buffer_size = nleaves/2;
  dmnsn_colored_prnode **buffer = dmnsn_malloc(buffer_size*sizeof(dmnsn_colored_prnode *));

  dmnsn_array *new_leaves = DMNSN_NEW_ARRAY(dmnsn_bvh_node *);

  dmnsn_priority_leaves_recursive(sorted_leaves, 0, nleaves, buffer, new_leaves, 0);

  dmnsn_free(buffer);
  for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
    dmnsn_free(sorted_leaves[i]);
  }
  dmnsn_free(colored_leaves);

  return new_leaves;
}

dmnsn_bvh_node *
dmnsn_new_prtree(const dmnsn_array *objects)
{
  if (dmnsn_array_size(objects) == 0) {
    return NULL;
  }

  // Make the initial array of leaves
  dmnsn_array *leaves = DMNSN_NEW_ARRAY(dmnsn_bvh_node *);
  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, objects) {
    dmnsn_bvh_node *node = dmnsn_new_bvh_leaf_node(*object);
    dmnsn_array_push(leaves, &node);
  }

  unsigned int ncpus = dmnsn_ncpus();
  unsigned int nthreads = ncpus < DMNSN_PSEUDO_B ? ncpus : DMNSN_PSEUDO_B;
  while (dmnsn_array_size(leaves) > 1) {
    dmnsn_array *new_leaves = dmnsn_priority_leaves(leaves, nthreads);
    dmnsn_delete_array(leaves);
    leaves = new_leaves;
  }

  dmnsn_bvh_node *root = *(dmnsn_bvh_node **)dmnsn_array_first(leaves);
  dmnsn_delete_array(leaves);
  return root;
}
