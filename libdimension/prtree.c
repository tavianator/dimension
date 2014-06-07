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

#include "dimension-internal.h"
#include <stdlib.h>

/// Number of children per PR-node.
#define DMNSN_PRTREE_B 8
/// Number of priority leaves per pseudo-PR-node (must be 2*ndimensions).
#define DMNSN_PSEUDO_B 6

/// The side of the split that a node ended up on.
typedef enum dmnsn_prnode_location {
  DMNSN_PRTREE_LEAF, ///< Priority leaf.
  DMNSN_PRTREE_LEFT, ///< Left child.
  DMNSN_PRTREE_RIGHT ///< Right child.
} dmnsn_prnode_location;

/// Construct an empty PR-node.
static inline dmnsn_bvh_node *
dmnsn_new_prnode(void)
{
  dmnsn_bvh_node *node = dmnsn_new_bvh_node(DMNSN_PRTREE_B);
  node->data = DMNSN_PRTREE_LEFT; // Mustn't be _LEAF
  return node;
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
  double lval = (*(const dmnsn_bvh_node **)l)->bounding_box.min.x;
  double rval = (*(const dmnsn_bvh_node **)r)->bounding_box.min.x;
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_ymin_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_bvh_node **)l)->bounding_box.min.y;
  double rval = (*(const dmnsn_bvh_node **)r)->bounding_box.min.y;
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_zmin_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_bvh_node **)l)->bounding_box.min.z;
  double rval = (*(const dmnsn_bvh_node **)r)->bounding_box.min.z;
  return (lval > rval) - (lval < rval);
}

static int
dmnsn_xmax_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_bvh_node **)l)->bounding_box.max.x;
  double rval = (*(const dmnsn_bvh_node **)r)->bounding_box.max.x;
  return (lval < rval) - (lval > rval);
}

static int
dmnsn_ymax_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_bvh_node **)l)->bounding_box.max.y;
  double rval = (*(const dmnsn_bvh_node **)r)->bounding_box.max.y;
  return (lval < rval) - (lval > rval);
}

static int
dmnsn_zmax_comp(const void *l, const void *r)
{
  double lval = (*(const dmnsn_bvh_node **)l)->bounding_box.max.z;
  double rval = (*(const dmnsn_bvh_node **)r)->bounding_box.max.z;
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
dmnsn_add_priority_leaves(dmnsn_bvh_node **sorted_leaves[DMNSN_PSEUDO_B],
                          size_t nleaves,
                          dmnsn_array *new_leaves)
{
  for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
    dmnsn_bvh_node *leaf = NULL;
    dmnsn_bvh_node **leaves = sorted_leaves[i];
    for (size_t j = 0;
         j < nleaves && (!leaf || leaf->nchildren < DMNSN_PRTREE_B);
         ++j)
    {
      // Skip all the previously found extreme nodes
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

static void
dmnsn_split_sorted_leaves_easy(dmnsn_bvh_node **leaves,
                               size_t *nleaves,
                               size_t *nright_leaves)
{
  // Get rid of the extreme nodes
  size_t i, skip, size = *nleaves;
  for (i = 0, skip = 0; i < size; ++i) {
    if (leaves[i]->data == DMNSN_PRTREE_LEAF) {
      ++skip;
    } else {
      leaves[i - skip] = leaves[i];
    }
  }

  size -= skip;

  // Split the leaves and mark the left and right child nodes
  size_t left_size = (size + 1)/2;
  for (i = 0; i < left_size; ++i) {
    leaves[i]->data = DMNSN_PRTREE_LEFT;
  }
  for (i = left_size; i < size; ++i) {
    leaves[i]->data = DMNSN_PRTREE_RIGHT;
  }

  *nleaves = left_size;
  *nright_leaves = size - left_size;
}

static void
dmnsn_split_sorted_leaves_hard(dmnsn_bvh_node **leaves,
                               dmnsn_bvh_node **buffer,
                               size_t nleaves)
{
  size_t i, j, skip;
  for (i = 0, j = 0, skip = 0; i < nleaves; ++i) {
    if (leaves[i]->data == DMNSN_PRTREE_LEFT) {
      leaves[i - skip] = leaves[i];
    } else {
      if (leaves[i]->data == DMNSN_PRTREE_RIGHT) {
        buffer[j] = leaves[i];
        ++j;
      }
      ++skip;
    }
  }

  size_t left_size = i - skip;
  for (i = 0; i < j; ++i) {
    leaves[left_size + i] = buffer[i];
  }
}

/// Split the sorted lists into the left and right subtrees.
static void
dmnsn_split_sorted_leaves(dmnsn_bvh_node **sorted_leaves[DMNSN_PSEUDO_B],
                          size_t *nleaves,
                          dmnsn_bvh_node **right_sorted_leaves[DMNSN_PSEUDO_B],
                          size_t *nright_leaves,
                          dmnsn_bvh_node **buffer,
                          size_t i)
{
  size_t original_size = *nleaves;

  // Split the ith list
  dmnsn_split_sorted_leaves_easy(sorted_leaves[i], nleaves, nright_leaves);

  // Split the rest of the lists
  for (size_t j = 0; j < DMNSN_PSEUDO_B; ++j) {
    right_sorted_leaves[j] = sorted_leaves[j] + *nleaves;
    if (j == i) {
      continue;
    }

    dmnsn_split_sorted_leaves_hard(sorted_leaves[j], buffer, original_size);
  }
}

/// Recursively constructs an implicit pseudo-PR-tree and collects the priority
/// leaves.
static void
dmnsn_priority_leaves_recursive(dmnsn_bvh_node **sorted_leaves[DMNSN_PSEUDO_B],
                                size_t nleaves,
                                dmnsn_bvh_node **buffer,
                                dmnsn_array *new_leaves,
                                int comparator)
{
  dmnsn_add_priority_leaves(sorted_leaves, nleaves, new_leaves);

  dmnsn_bvh_node **right_sorted_leaves[DMNSN_PSEUDO_B];
  size_t right_nleaves;

  dmnsn_split_sorted_leaves(sorted_leaves, &nleaves,
                            right_sorted_leaves, &right_nleaves,
                            buffer, comparator);

  if (nleaves > 0) {
    dmnsn_priority_leaves_recursive(sorted_leaves, nleaves,
                                    buffer, new_leaves,
                                    (comparator + 1)%DMNSN_PSEUDO_B);
  }

  if (right_nleaves > 0) {
    dmnsn_priority_leaves_recursive(right_sorted_leaves, right_nleaves,
                                    buffer, new_leaves,
                                    (comparator + 1)%DMNSN_PSEUDO_B);
  }
}

/// Sort each dimension in parallel with more than this many leaves.
#define DMNSN_PARALLEL_SORT_THRESHOLD 1024

typedef struct {
  dmnsn_bvh_node **leaves_arr;
  dmnsn_bvh_node ***sorted_leaves;
  size_t nleaves;
} dmnsn_sort_leaves_payload;

static dmnsn_bvh_node **
dmnsn_sort_leaf_array(dmnsn_bvh_node **leaves, size_t nleaves, int comparator)
{
  size_t leaves_size = nleaves*sizeof(dmnsn_bvh_node *);
  dmnsn_bvh_node **sorted_leaves = dmnsn_malloc(leaves_size);
  memcpy(sorted_leaves, leaves, leaves_size);
  qsort(sorted_leaves, nleaves, sizeof(dmnsn_bvh_node *),
        dmnsn_comparators[comparator]);
  return sorted_leaves;
}

static int
dmnsn_sort_leaves(void *ptr, unsigned int thread, unsigned int nthreads)
{
  dmnsn_sort_leaves_payload *payload = ptr;

  for (unsigned int i = thread; i < DMNSN_PSEUDO_B; i += nthreads) {
    payload->sorted_leaves[i] =
      dmnsn_sort_leaf_array(payload->leaves_arr, payload->nleaves, i);
  }

  return 0;
}

/// Constructs an implicit pseudo-PR-tree and returns the priority leaves.
static dmnsn_array *
dmnsn_priority_leaves(const dmnsn_array *leaves, unsigned int nthreads)
{
  dmnsn_bvh_node **leaves_arr = dmnsn_array_first(leaves);
  dmnsn_bvh_node **sorted_leaves[DMNSN_PSEUDO_B];
  size_t nleaves = dmnsn_array_size(leaves);

  if (nleaves >= DMNSN_PARALLEL_SORT_THRESHOLD && nthreads > 1) {
    dmnsn_sort_leaves_payload payload = {
      .leaves_arr = leaves_arr,
      .sorted_leaves = sorted_leaves,
      .nleaves = nleaves,
    };
    dmnsn_execute_concurrently(NULL, dmnsn_sort_leaves, &payload, nthreads);
  } else {
    for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
      sorted_leaves[i] = dmnsn_sort_leaf_array(leaves_arr, nleaves, i);
    }
  }

  size_t buffer_size = nleaves/2;
  dmnsn_bvh_node **buffer = dmnsn_malloc(buffer_size*sizeof(dmnsn_bvh_node *));

  dmnsn_array *new_leaves = DMNSN_NEW_ARRAY(dmnsn_bvh_node *);

  dmnsn_priority_leaves_recursive(sorted_leaves, nleaves, buffer, new_leaves,
                                  0);

  dmnsn_free(buffer);
  for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
    dmnsn_free(sorted_leaves[i]);
  }

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
    node->data = DMNSN_PRTREE_LEFT; // Mustn't be _LEAF
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
