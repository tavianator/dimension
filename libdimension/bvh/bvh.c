/*************************************************************************
 * Copyright (C) 2012-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * BVH implementation.  These are the hottest code paths in libdimension.
 */

#include "internal/bvh.h"
#include "internal/concurrency.h"
#include "internal/prtree.h"
#include <pthread.h>

/// Implementation for DMNSN_BVH_NONE: just stick all objects in one node.
static dmnsn_bvh_node *
dmnsn_new_stupid_bvh(const dmnsn_array *objects)
{
  dmnsn_bvh_node *root = dmnsn_new_bvh_node(dmnsn_array_size(objects));

  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, objects) {
    dmnsn_bvh_node *leaf = dmnsn_new_bvh_leaf_node(*object);
    dmnsn_bvh_node_add(root, leaf);
  }

  return root;
}

// Implementation of opaque dmnsn_bvh type.
struct dmnsn_bvh {
  dmnsn_array *unbounded;           ///< The unbounded objects.
  dmnsn_array *bounded;             ///< The BVH of the bounded objects.
  pthread_key_t intersection_cache; ///< The thread-local intersection cache.
};

/// A flat BVH node for storing in an array for fast pre-order traversal.
typedef struct dmnsn_flat_bvh_node {
  dmnsn_aabb aabb;      ///< The bounding box of this node.
  dmnsn_object *object; ///< The referenced object, for leaf nodes.
  ptrdiff_t skip;       ///< Displacement to the next sibling.
} dmnsn_flat_bvh_node;

/// Add an object or its children, if any, to an array.
static void
dmnsn_split_add_object(dmnsn_array *objects, const dmnsn_object *object)
{
  if (object->split_children) {
    DMNSN_ARRAY_FOREACH (const dmnsn_object **, child, object->children) {
      dmnsn_split_add_object(objects, *child);
    }
  } else {
    dmnsn_array_push(objects, &object);
  }
}

/// Split unions to create the input for the BVH.
static dmnsn_array *
dmnsn_split_objects(const dmnsn_array *objects)
{
  dmnsn_array *split = DMNSN_NEW_ARRAY(dmnsn_object *);
  DMNSN_ARRAY_FOREACH (const dmnsn_object **, object, objects) {
    dmnsn_split_add_object(split, *object);
  }
  return split;
}

/// Split unbounded objects into a new array.
static dmnsn_array *
dmnsn_split_unbounded(dmnsn_array *objects)
{
  dmnsn_array *unbounded = DMNSN_NEW_ARRAY(dmnsn_object *);

  dmnsn_object **array = dmnsn_array_first(objects);
  size_t i, skip;
  for (i = 0, skip = 0; i < dmnsn_array_size(objects); ++i) {
    if (dmnsn_aabb_is_infinite(array[i]->aabb)) {
      dmnsn_array_push(unbounded, &array[i]);
      ++skip;
    } else {
      array[i - skip] = array[i];
    }
  }
  dmnsn_array_resize(objects, i - skip);

  return unbounded;
}

/// Recursively flatten a BVH into an array of flat nodes.
static void
dmnsn_flatten_bvh_recursive(dmnsn_bvh_node *node, dmnsn_array *flat)
{
  size_t currenti = dmnsn_array_size(flat);
  dmnsn_array_resize(flat, currenti + 1);
  dmnsn_flat_bvh_node *flatnode = dmnsn_array_at(flat, currenti);

  flatnode->aabb = node->aabb;
  flatnode->object = node->object;

  for (size_t i = 0; i < node->nchildren && node->children[i]; ++i) {
    dmnsn_flatten_bvh_recursive(node->children[i], flat);
  }

  // Array could have been realloc()'d somewhere else above
  flatnode = dmnsn_array_at(flat, currenti);
  flatnode->skip = dmnsn_array_size(flat) - currenti;
}

/// Flatten a BVH into an array of flat nodes.
static dmnsn_array *
dmnsn_flatten_bvh(dmnsn_bvh_node *root)
{
  dmnsn_array *flat = DMNSN_NEW_ARRAY(dmnsn_flat_bvh_node);
  if (root) {
    dmnsn_flatten_bvh_recursive(root, flat);
  }
  return flat;
}

dmnsn_bvh *dmnsn_new_bvh(const dmnsn_array *objects, dmnsn_bvh_kind kind)
{
  dmnsn_bvh *bvh = DMNSN_MALLOC(dmnsn_bvh);

  dmnsn_array *bounded = dmnsn_split_objects(objects);
  bvh->unbounded = dmnsn_split_unbounded(bounded);

  dmnsn_bvh_node *root = NULL;
  if (dmnsn_array_size(bounded) > 0) {
    switch (kind) {
    case DMNSN_BVH_NONE:
      root = dmnsn_new_stupid_bvh(bounded);
      break;
    case DMNSN_BVH_PRTREE:
      root = dmnsn_new_prtree(bounded);
      break;
    default:
      dmnsn_unreachable("Invalid BVH kind.");
    }
  }
  bvh->bounded = dmnsn_flatten_bvh(root);

  dmnsn_delete_bvh_node(root);
  dmnsn_delete_array(bounded);

  dmnsn_key_create(&bvh->intersection_cache, dmnsn_free);

  return bvh;
}

void
dmnsn_delete_bvh(dmnsn_bvh *bvh)
{
  if (bvh) {
    dmnsn_free(pthread_getspecific(bvh->intersection_cache));
    dmnsn_key_delete(bvh->intersection_cache);
    dmnsn_delete_array(bvh->bounded);
    dmnsn_delete_array(bvh->unbounded);
    dmnsn_free(bvh);
  }
}

/// A ray with pre-calculated reciprocals to avoid divisions.
typedef struct dmnsn_optimized_ray {
  dmnsn_vector x0;    ///< The origin of the ray.
  dmnsn_vector n_inv; ///< The inverse of each component of the ray's slope
} dmnsn_optimized_ray;

/// Precompute inverses for faster ray-box intersection tests.
static inline dmnsn_optimized_ray
dmnsn_optimize_ray(dmnsn_ray ray)
{
  dmnsn_optimized_ray optray = {
    .x0    = ray.x0,
    .n_inv = dmnsn_new_vector(1.0/ray.n.x, 1.0/ray.n.y, 1.0/ray.n.z)
  };
  return optray;
}

/// Ray-AABB intersection test, by the slab method.  Highly optimized.
static inline bool
dmnsn_ray_box_intersection(dmnsn_optimized_ray optray, dmnsn_aabb box, double t)
{
  // This is actually correct, even though it appears not to handle edge cases
  // (ray.n.{x,y,z} == 0).  It works because the infinities that result from
  // dividing by zero will still behave correctly in the comparisons.  Rays
  // which are parallel to an axis and outside the box will have tmin == inf
  // or tmax == -inf, while rays inside the box will have tmin and tmax
  // unchanged.

  double tx1 = (box.min.x - optray.x0.x)*optray.n_inv.x;
  double tx2 = (box.max.x - optray.x0.x)*optray.n_inv.x;

  double tmin = dmnsn_min(tx1, tx2);
  double tmax = dmnsn_max(tx1, tx2);

  double ty1 = (box.min.y - optray.x0.y)*optray.n_inv.y;
  double ty2 = (box.max.y - optray.x0.y)*optray.n_inv.y;

  tmin = dmnsn_max(tmin, dmnsn_min(ty1, ty2));
  tmax = dmnsn_min(tmax, dmnsn_max(ty1, ty2));

  double tz1 = (box.min.z - optray.x0.z)*optray.n_inv.z;
  double tz2 = (box.max.z - optray.x0.z)*optray.n_inv.z;

  tmin = dmnsn_max(tmin, dmnsn_min(tz1, tz2));
  tmax = dmnsn_min(tmax, dmnsn_max(tz1, tz2));

  return tmax >= dmnsn_max(0.0, tmin) && tmin < t;
}

/// The number of intersections to cache.
#define DMNSN_INTERSECTION_CACHE_SIZE 32

/// An array of cached intersections.
typedef struct dmnsn_intersection_cache {
  size_t i;
  dmnsn_object *objects[DMNSN_INTERSECTION_CACHE_SIZE];
} dmnsn_intersection_cache;

static dmnsn_intersection_cache *
dmnsn_get_intersection_cache(const dmnsn_bvh *bvh)
{
  dmnsn_intersection_cache *cache
    = pthread_getspecific(bvh->intersection_cache);

  if (!cache) {
    cache = DMNSN_MALLOC(dmnsn_intersection_cache);
    cache->i = 0;
    for (size_t i = 0; i < DMNSN_INTERSECTION_CACHE_SIZE; ++i) {
      cache->objects[i] = NULL;
    }
    dmnsn_setspecific(bvh->intersection_cache, cache);
  }

  return cache;
}

/// Test for a closer object intersection than we've found so far.
static inline bool
dmnsn_closer_intersection(dmnsn_object *object, dmnsn_ray ray, dmnsn_intersection *intersection, double *t)
{
  dmnsn_intersection local_intersection;
  if (dmnsn_object_intersection(object, ray, &local_intersection)) {
    if (local_intersection.t < *t) {
      *intersection = local_intersection;
      *t = local_intersection.t;
      return true;
    }
  }
  return false;
}

DMNSN_HOT bool
dmnsn_bvh_intersection(const dmnsn_bvh *bvh, dmnsn_ray ray, dmnsn_intersection *intersection, bool reset)
{
  double t = INFINITY;

  // Search the unbounded objects
  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, bvh->unbounded) {
    dmnsn_closer_intersection(*object, ray, intersection, &t);
  }

  // Precalculate 1.0/ray.n.{x,y,z} to save time in intersection tests
  dmnsn_optimized_ray optray = dmnsn_optimize_ray(ray);

  // Search the intersection cache
  dmnsn_intersection_cache *cache = dmnsn_get_intersection_cache(bvh);
  if (dmnsn_unlikely(reset)) {
    cache->i = 0;
  }
  dmnsn_object *cached = NULL, *found = NULL;
  if (dmnsn_likely(cache->i < DMNSN_INTERSECTION_CACHE_SIZE)) {
    cached = cache->objects[cache->i];
  }
  if (cached && dmnsn_ray_box_intersection(optray, cached->aabb, t)) {
    if (dmnsn_closer_intersection(cached, ray, intersection, &t)) {
      found = cached;
    }
  }

  // Search the bounded objects
  dmnsn_flat_bvh_node *node = dmnsn_array_first(bvh->bounded);
  dmnsn_flat_bvh_node *last = dmnsn_array_last(bvh->bounded);
  while (node <= last) {
    if (dmnsn_ray_box_intersection(optray, node->aabb, t)) {
      if (node->object && node->object != cached) {
        if (dmnsn_closer_intersection(node->object, ray, intersection, &t)) {
          found = node->object;
        }
      }
      ++node;
    } else {
      node += node->skip;
    }
  }

  // Update the cache
  if (dmnsn_likely(cache->i < DMNSN_INTERSECTION_CACHE_SIZE)) {
    cache->objects[cache->i] = found;
    ++cache->i;
  }

  return !isinf(t);
}

DMNSN_HOT bool
dmnsn_bvh_inside(const dmnsn_bvh *bvh, dmnsn_vector point)
{
  // Search the unbounded objects
  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, bvh->unbounded) {
    if (dmnsn_object_inside(*object, point))
      return true;
  }

  // Search the bounded objects
  dmnsn_flat_bvh_node *node = dmnsn_array_first(bvh->bounded);
  dmnsn_flat_bvh_node *last = dmnsn_array_last(bvh->bounded);
  while (node <= last) {
    if (dmnsn_aabb_contains(node->aabb, point)) {
      if (node->object && dmnsn_object_inside(node->object, point)) {
        return true;
      }
      ++node;
    } else {
      node += node->skip;
    }
  }

  return false;
}

dmnsn_aabb
dmnsn_bvh_aabb(const dmnsn_bvh *bvh)
{
  if (dmnsn_array_size(bvh->unbounded) > 0) {
    return dmnsn_infinite_aabb();
  } else if (dmnsn_array_size(bvh->bounded) > 0) {
    dmnsn_flat_bvh_node *root = dmnsn_array_first(bvh->bounded);
    return root->aabb;
  } else {
    return dmnsn_zero_aabb();
  }
}

dmnsn_bvh_node *
dmnsn_new_bvh_node(unsigned int max_children)
{
  dmnsn_bvh_node *node = dmnsn_malloc(sizeof(dmnsn_bvh_node) + max_children*sizeof(dmnsn_bvh_node *));
  node->aabb = dmnsn_zero_aabb();
  node->object = NULL;
  node->nchildren = 0;
  node->max_children = max_children;
  return node;
}

dmnsn_bvh_node *
dmnsn_new_bvh_leaf_node(dmnsn_object *object)
{
  dmnsn_bvh_node *node = DMNSN_MALLOC(dmnsn_bvh_node);
  node->aabb = object->aabb;
  node->object = object;
  node->nchildren = 0;
  node->max_children = 0;
  return node;
}

void
dmnsn_delete_bvh_node(dmnsn_bvh_node *node)
{
  if (node) {
    for (size_t i = 0; i < node->nchildren; ++i) {
      dmnsn_delete_bvh_node(node->children[i]);
    }
    dmnsn_free(node);
  }
}

void
dmnsn_bvh_node_add(dmnsn_bvh_node *parent, dmnsn_bvh_node *child)
{
  dmnsn_assert(parent->nchildren < parent->max_children,
               "Too many BVH children inserted.");

  parent->aabb.min = dmnsn_vector_min(parent->aabb.min, child->aabb.min);
  parent->aabb.max = dmnsn_vector_max(parent->aabb.max, child->aabb.max);
  parent->children[parent->nchildren++] = child;
}
