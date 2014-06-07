/*************************************************************************
 * Copyright (C) 2012 Tavian Barnes <tavianator@tavianator.com>          *
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
 * @file.
 * Bounding volume hierarchy.  This generic interface allows different data
 * structures to be represented in the same way, thus sharing code for their
 * traversal algorithm.
 */

#include <stdbool.h>

/// A bounding volume hierarchy.
typedef struct dmnsn_bvh dmnsn_bvh;

/// Available BVH implementations.
typedef enum dmnsn_bvh_kind {
  DMNSN_BVH_NONE,
  DMNSN_BVH_PRTREE,
} dmnsn_bvh_kind;

/// Create a BVH.
DMNSN_INTERNAL dmnsn_bvh *dmnsn_new_bvh(const dmnsn_array *objects,
                                        dmnsn_bvh_kind kind);
/// Delete a BVH.
DMNSN_INTERNAL void dmnsn_delete_bvh(dmnsn_bvh *bvh);

/// Find the closest ray-object intersection in the tree.
DMNSN_INTERNAL bool dmnsn_bvh_intersection(const dmnsn_bvh *bvh,
                                           dmnsn_line ray,
                                           dmnsn_intersection *intersection,
                                           bool reset);
/// Determine whether a point is inside any object in the tree.
DMNSN_INTERNAL bool dmnsn_bvh_inside(const dmnsn_bvh *bvh, dmnsn_vector point);
/// Return the bounding box of the whole hierarchy.
DMNSN_INTERNAL dmnsn_bounding_box dmnsn_bvh_bounding_box(const dmnsn_bvh *bvh);

/// A non-flat BVH representation, used by BVH implementations.
typedef struct dmnsn_bvh_node {
  dmnsn_bounding_box bounding_box;   /// The bounding box of this node.
  dmnsn_object *object;              /// The object, for leaf nodes.
  int data;                          /// Extra field for implementation use.
  size_t nchildren;                  /// How many children this node has.
  size_t max_children;               /// Maximum number of children.
  struct dmnsn_bvh_node *children[]; /// Flexible array of children.
} dmnsn_bvh_node;

/// Create a BVH node.
DMNSN_INTERNAL dmnsn_bvh_node *dmnsn_new_bvh_node(size_t max_children);

/// Create a BVH leaf node.
DMNSN_INTERNAL dmnsn_bvh_node *dmnsn_new_bvh_leaf_node(dmnsn_object *object);

/// Delete a BVH node.
DMNSN_INTERNAL void dmnsn_delete_bvh_node(dmnsn_bvh_node *node);

/// Add a child to a BVH node.
DMNSN_INTERNAL void dmnsn_bvh_node_add(dmnsn_bvh_node *parent,
                                       dmnsn_bvh_node *child);
