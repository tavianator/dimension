/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Priority R-tree implementation.  These are the hottest code paths in
 * libdimension.
 */

#include "dimension-impl.h"
#include <pthread.h>
#include <stdlib.h>

/** Number of children per PR-node. */
#define DMNSN_PRTREE_B 8
/** Number of priority leaves per pseudo-PR-node (must be 2*ndimensions). */
#define DMNSN_PSEUDO_B 6

/** A flat node for storing in an array for fast pre-order traversal. */
typedef struct dmnsn_flat_prnode {
  dmnsn_bounding_box bounding_box;
  dmnsn_object *object;
  size_t skip;
} dmnsn_flat_prnode;

/** The side of the split that a node ended up on. */
typedef enum dmnsn_prnode_location {
  DMNSN_PRTREE_LEAF, /**< Priority leaf. */
  DMNSN_PRTREE_LEFT, /**< Left child. */
  DMNSN_PRTREE_RIGHT /**< Right child. */
} dmnsn_prnode_location;

/** Pseudo PR-tree node. */
typedef struct dmnsn_prnode {
  dmnsn_bounding_box bounding_box;

  dmnsn_object *object;
  struct dmnsn_prnode *children[DMNSN_PRTREE_B];

  dmnsn_prnode_location location;
} dmnsn_prnode;

/** Construct an empty PR-node. */
static inline dmnsn_prnode *
dmnsn_new_prnode()
{
  dmnsn_prnode *node = dmnsn_malloc(sizeof(dmnsn_prnode));
  node->bounding_box = dmnsn_zero_bounding_box();
  node->object       = NULL;
  node->location     = DMNSN_PRTREE_LEFT; /* Mustn't be _LEAF */
  for (size_t i = 0; i < DMNSN_PRTREE_B; ++i) {
    node->children[i] = NULL;
  }
  return node;
}

/** Free a non-flat PR-tree. */
static void
dmnsn_delete_prnode(dmnsn_prnode *node)
{
  if (node) {
    for (size_t i = 0; i < DMNSN_PRTREE_B && node->children[i]; ++i) {
      dmnsn_delete_prnode(node->children[i]);
    }
    dmnsn_free(node);
  }
}

/** Expand a node to contain the bounding box \p box. */
static void
dmnsn_prnode_swallow(dmnsn_prnode *node, dmnsn_bounding_box box)
{
  node->bounding_box.min = dmnsn_vector_min(node->bounding_box.min, box.min);
  node->bounding_box.max = dmnsn_vector_max(node->bounding_box.max, box.max);
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
dmnsn_get_coordinate(const dmnsn_list_iterator *i, int comparator)
{
  dmnsn_prnode **node = dmnsn_list_at(i);

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
    dmnsn_assert(false, "Invalid comparator.");
    return 0.0; /* Shut up compiler */
  }
}

/* List sorting comparators */

static bool
dmnsn_xmin_comp(const dmnsn_list_iterator *l, const dmnsn_list_iterator *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_XMIN);
  double rval = dmnsn_get_coordinate(r, DMNSN_XMIN);
  return lval < rval;
}

static bool
dmnsn_ymin_comp(const dmnsn_list_iterator *l, const dmnsn_list_iterator *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_YMIN);
  double rval = dmnsn_get_coordinate(r, DMNSN_YMIN);
  return lval < rval;
}

static bool
dmnsn_zmin_comp(const dmnsn_list_iterator *l, const dmnsn_list_iterator *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_ZMIN);
  double rval = dmnsn_get_coordinate(r, DMNSN_ZMIN);
  return lval < rval;
}

static bool
dmnsn_xmax_comp(const dmnsn_list_iterator *l, const dmnsn_list_iterator *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_XMAX);
  double rval = dmnsn_get_coordinate(r, DMNSN_XMAX);
  return lval < rval;
}

static bool
dmnsn_ymax_comp(const dmnsn_list_iterator *l, const dmnsn_list_iterator *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_YMAX);
  double rval = dmnsn_get_coordinate(r, DMNSN_YMAX);
  return lval < rval;
}

static bool
dmnsn_zmax_comp(const dmnsn_list_iterator *l, const dmnsn_list_iterator *r)
{
  double lval = dmnsn_get_coordinate(l, DMNSN_ZMAX);
  double rval = dmnsn_get_coordinate(r, DMNSN_ZMAX);
  return lval < rval;
}

/** All comparators. */
static dmnsn_list_comparator_fn *const dmnsn_comparators[DMNSN_PSEUDO_B] = {
  [DMNSN_XMIN] = dmnsn_xmin_comp,
  [DMNSN_YMIN] = dmnsn_ymin_comp,
  [DMNSN_ZMIN] = dmnsn_zmin_comp,
  [DMNSN_XMAX] = dmnsn_xmax_comp,
  [DMNSN_YMAX] = dmnsn_ymax_comp,
  [DMNSN_ZMAX] = dmnsn_zmax_comp
};

/** Add the priority leaves for this level. */
static void
dmnsn_add_priority_leaves(dmnsn_list *sorted_leaves[DMNSN_PSEUDO_B],
                          dmnsn_list *new_leaves, int comparator)
{
  for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
    dmnsn_prnode *leaf = dmnsn_new_prnode();
    size_t count = 0;
    dmnsn_list_iterator *j = dmnsn_list_first(sorted_leaves[i]);
    while (count < DMNSN_PRTREE_B && j) {
      /* Skip all the previously found extreme nodes */
      dmnsn_prnode **node;
      do {
        node = dmnsn_list_at(j);
        j = dmnsn_list_next(j);
      } while ((*node)->location == DMNSN_PRTREE_LEAF && j);

      if ((*node)->location != DMNSN_PRTREE_LEAF) {
        (*node)->location = DMNSN_PRTREE_LEAF;
        leaf->children[count++] = *node;
        dmnsn_prnode_swallow(leaf, (*node)->bounding_box);
      }
    }

    if (count > 0) {
      dmnsn_list_push(new_leaves, &leaf);
    } else {
      dmnsn_delete_prnode(leaf);
      return;
    }
  }
}

/** Split the sorted lists into the left and right subtrees. */
static bool
dmnsn_split_sorted_leaves(dmnsn_list *sorted_leaves[DMNSN_PSEUDO_B],
                          dmnsn_list *right_sorted_leaves[DMNSN_PSEUDO_B],
                          size_t i)
{
  /* Get rid of the extreme nodes */
  dmnsn_list_iterator *j = dmnsn_list_first(sorted_leaves[i]);
  while (j) {
    dmnsn_list_iterator *next = dmnsn_list_next(j);
    dmnsn_prnode **node = dmnsn_list_at(j);
    if ((*node)->location == DMNSN_PRTREE_LEAF) {
      dmnsn_list_remove(sorted_leaves[i], j);
    }
    j = next;
  }

  if (dmnsn_list_size(sorted_leaves[i]) == 0) {
    return false;
  }

  /* Split the appropriate list and mark the left and right child nodes */
  right_sorted_leaves[i] = dmnsn_list_split(sorted_leaves[i]);
  for (dmnsn_list_iterator *j = dmnsn_list_first(sorted_leaves[i]);
       j != NULL;
       j = dmnsn_list_next(j))
  {
    dmnsn_prnode **node = dmnsn_list_at(j);
    (*node)->location = DMNSN_PRTREE_LEFT;
  }
  for (dmnsn_list_iterator *j = dmnsn_list_first(right_sorted_leaves[i]);
       j != NULL;
       j = dmnsn_list_next(j))
  {
    dmnsn_prnode **node = dmnsn_list_at(j);
    (*node)->location = DMNSN_PRTREE_RIGHT;
  }

  /* Split the rest of the lists */
  for (size_t j = 0; j < DMNSN_PSEUDO_B; ++j) {
    if (j != i) {
      right_sorted_leaves[j] = dmnsn_new_list(sizeof(dmnsn_prnode *));

      dmnsn_list_iterator *k = dmnsn_list_first(sorted_leaves[j]);
      while (k) {
        dmnsn_list_iterator *next = dmnsn_list_next(k);
        dmnsn_prnode **node = dmnsn_list_at(k);
        if ((*node)->location == DMNSN_PRTREE_LEAF) {
          dmnsn_list_remove(sorted_leaves[j], k);
        } else if ((*node)->location == DMNSN_PRTREE_RIGHT) {
          dmnsn_list_iterator_remove(sorted_leaves[j], k);
          dmnsn_list_iterator_insert(right_sorted_leaves[j], NULL, k);
        }
        k = next;
      }
    }
  }

  return true;
}

/** Recursively constructs an implicit pseudo-PR-tree and collects the priority
    leaves. */
static void
dmnsn_priority_leaves_recursive(dmnsn_list *sorted_leaves[DMNSN_PSEUDO_B],
                                dmnsn_list *new_leaves,
                                int comparator)
{
  dmnsn_add_priority_leaves(sorted_leaves, new_leaves, comparator);

  dmnsn_list *right_sorted_leaves[DMNSN_PSEUDO_B];
  if (dmnsn_split_sorted_leaves(sorted_leaves, right_sorted_leaves, comparator))
  {
    dmnsn_priority_leaves_recursive(right_sorted_leaves, new_leaves,
                                    (comparator + 1)%DMNSN_PSEUDO_B);
    for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
      dmnsn_delete_list(right_sorted_leaves[i]);
    }

    dmnsn_priority_leaves_recursive(sorted_leaves, new_leaves,
                                    (comparator + 1)%DMNSN_PSEUDO_B);
  }
}

/** Constructs an implicit pseudo-PR-tree and returns the priority leaves. */
static dmnsn_list *
dmnsn_priority_leaves(const dmnsn_list *leaves)
{
  dmnsn_list *sorted_leaves[DMNSN_PSEUDO_B];
  for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
    sorted_leaves[i] = dmnsn_copy_list(leaves);
    dmnsn_list_sort(sorted_leaves[i], dmnsn_comparators[i]);
  }

  dmnsn_list *new_leaves = dmnsn_new_list(sizeof(dmnsn_prnode *));
  dmnsn_priority_leaves_recursive(sorted_leaves, new_leaves, 0);

  for (size_t i = 0; i < DMNSN_PSEUDO_B; ++i) {
    dmnsn_delete_list(sorted_leaves[i]);
  }

  return new_leaves;
}

/** Construct a non-flat PR-tree. */
static dmnsn_prnode *
dmnsn_make_prtree(const dmnsn_list *objects)
{
  if (dmnsn_list_size(objects) == 0) {
    return NULL;
  }

  /* Make the initial list of leaves */
  dmnsn_list *leaves = dmnsn_new_list(sizeof(dmnsn_prnode *));
  for (dmnsn_list_iterator *i = dmnsn_list_first(objects);
       i != NULL;
       i = dmnsn_list_next(i))
  {
    dmnsn_object **object = dmnsn_list_at(i);
    dmnsn_prnode *node = dmnsn_new_prnode();
    node->bounding_box = (*object)->bounding_box;
    node->object       = *object;
    dmnsn_list_push(leaves, &node);
  }

  while (dmnsn_list_size(leaves) > 1) {
    dmnsn_list *new_leaves = dmnsn_priority_leaves(leaves);
    dmnsn_delete_list(leaves);
    leaves = new_leaves;
  }

  dmnsn_prnode **leaf = dmnsn_list_at(dmnsn_list_first(leaves));
  dmnsn_prnode *root = *leaf;
  dmnsn_delete_list(leaves);
  return root;
}

/** Add an object or its children, if any, to a list. */
static void
dmnsn_list_add_object(dmnsn_list *objects, const dmnsn_object *object)
{
  if (dmnsn_array_size(object->children) == 0) {
    dmnsn_list_push(objects, &object);
  } else {
    DMNSN_ARRAY_FOREACH (const dmnsn_object **, child, object->children) {
      dmnsn_list_add_object(objects, *child);
    }
  }
}

/** Add objects from an array to a list, splitting unions etc. */
static dmnsn_list *
dmnsn_object_list(const dmnsn_array *objects)
{
  dmnsn_list *list = dmnsn_new_list(sizeof(dmnsn_object *));
  DMNSN_ARRAY_FOREACH (const dmnsn_object **, object, objects) {
    dmnsn_list_add_object(list, *object);
  }
  return list;
}

/** Split unbounded objects into a new list. */
static dmnsn_list *
dmnsn_split_unbounded(dmnsn_list *objects)
{
  dmnsn_list *unbounded = dmnsn_new_list(sizeof(dmnsn_object *));

  dmnsn_list_iterator *i = dmnsn_list_first(objects);
  while (i) {
    dmnsn_object **object = dmnsn_list_at(i);

    if (dmnsn_bounding_box_is_infinite((*object)->bounding_box)) {
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

/** Recursively flatten a PR-tree into an array of flat nodes. */
static void
dmnsn_flatten_prtree_recursive(dmnsn_prnode *node, dmnsn_array *flat)
{
  size_t currenti = dmnsn_array_size(flat);
  dmnsn_array_resize(flat, currenti + 1);
  dmnsn_flat_prnode *flatnode = dmnsn_array_at(flat, currenti);

  flatnode->bounding_box = node->bounding_box;
  flatnode->object       = node->object;

  for (size_t i = 0; i < DMNSN_PRTREE_B && node->children[i]; ++i) {
    dmnsn_flatten_prtree_recursive(node->children[i], flat);
  }

  /* Array could have been realloc()'d somewhere else above */
  flatnode = dmnsn_array_at(flat, currenti);
  flatnode->skip = dmnsn_array_size(flat) - currenti;
}

/** Flatten a PR-tree into an array of flat nodes. */
static dmnsn_array *
dmnsn_flatten_prtree(dmnsn_prnode *root)
{
  dmnsn_array *flat = dmnsn_new_array(sizeof(dmnsn_flat_prnode));
  if (root) {
    dmnsn_flatten_prtree_recursive(root, flat);
  }
  return flat;
}

static size_t dmnsn_prtree_seq = 0;
static pthread_mutex_t dmnsn_prtree_seq_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Construct a PR-tree from a bulk of objects */
dmnsn_prtree *
dmnsn_new_prtree(const dmnsn_array *objects)
{
  dmnsn_prtree *prtree = dmnsn_malloc(sizeof(dmnsn_prtree));

  dmnsn_list *object_list = dmnsn_object_list(objects);

  dmnsn_list *unbounded = dmnsn_split_unbounded(object_list);
  prtree->unbounded = dmnsn_array_from_list(unbounded);

  dmnsn_prnode *root = dmnsn_make_prtree(object_list);
  prtree->bounded = dmnsn_flatten_prtree(root);

  dmnsn_delete_prnode(root);
  dmnsn_delete_list(unbounded);
  dmnsn_delete_list(object_list);

  if (dmnsn_array_size(prtree->unbounded) > 0) {
    prtree->bounding_box = dmnsn_infinite_bounding_box();
  } else if (dmnsn_array_size(prtree->bounded) > 0) {
    dmnsn_flat_prnode *root = dmnsn_array_first(prtree->bounded);
    prtree->bounding_box = root->bounding_box;
  } else {
    prtree->bounding_box = dmnsn_zero_bounding_box();
  }

  if (pthread_mutex_lock(&dmnsn_prtree_seq_mutex) != 0) {
    dmnsn_error("Couldn't lock mutex.");
  }
  prtree->id = dmnsn_prtree_seq++;
  if (pthread_mutex_unlock(&dmnsn_prtree_seq_mutex) != 0) {
    dmnsn_error("Couldn't unlock mutex.");
  }

  return prtree;
}

/** Free a PR-tree. */
void
dmnsn_delete_prtree(dmnsn_prtree *tree)
{
  if (tree) {
    dmnsn_delete_array(tree->bounded);
    dmnsn_delete_array(tree->unbounded);
    dmnsn_free(tree);
  }
}

/** A line with pre-calculated reciprocals to avoid divisions. */
typedef struct dmnsn_optimized_line {
  dmnsn_vector x0;    /**< The origin of the line. */
  dmnsn_vector n_inv; /**< The inverse of each component of the line's slope .*/
} dmnsn_optimized_line;

/** Precompute inverses for faster ray-box intersection tests. */
static inline dmnsn_optimized_line
dmnsn_optimize_line(dmnsn_line line)
{
  dmnsn_optimized_line optline = {
    .x0    = line.x0,
    .n_inv = dmnsn_new_vector(1.0/line.n.x, 1.0/line.n.y, 1.0/line.n.z)
  };
  return optline;
}

/** Ray-AABB intersection test, by the slab method.  Highly optimized. */
static inline bool
dmnsn_ray_box_intersection(dmnsn_optimized_line optline,
                           dmnsn_bounding_box box, double t)
{
  /*
   * This is actually correct, even though it appears not to handle edge cases
   * (line.n.{x,y,z} == 0).  It works because the infinities that result from
   * dividing by zero will still behave correctly in the comparisons.  Lines
   * which are parallel to an axis and outside the box will have tmin == inf
   * or tmax == -inf, while lines inside the box will have tmin and tmax
   * unchanged.
   */

  double tx1 = (box.min.x - optline.x0.x)*optline.n_inv.x;
  double tx2 = (box.max.x - optline.x0.x)*optline.n_inv.x;

  double tmin = dmnsn_min(tx1, tx2);
  double tmax = dmnsn_max(tx1, tx2);

  double ty1 = (box.min.y - optline.x0.y)*optline.n_inv.y;
  double ty2 = (box.max.y - optline.x0.y)*optline.n_inv.y;

  tmin = dmnsn_max(tmin, dmnsn_min(ty1, ty2));
  tmax = dmnsn_min(tmax, dmnsn_max(ty1, ty2));

  double tz1 = (box.min.z - optline.x0.z)*optline.n_inv.z;
  double tz2 = (box.max.z - optline.x0.z)*optline.n_inv.z;

  tmin = dmnsn_max(tmin, dmnsn_min(tz1, tz2));
  tmax = dmnsn_min(tmax, dmnsn_max(tz1, tz2));

  return tmax >= dmnsn_max(0.0, tmin) && tmin < t;
}

/** The number of intersections to cache. */
#define DMNSN_PRTREE_CACHE_SIZE 32

/** An array of cached intersections. */
typedef struct dmnsn_intersection_cache {
  size_t i;
  dmnsn_object *objects[DMNSN_PRTREE_CACHE_SIZE];
} dmnsn_intersection_cache;

/** The thread-specific intersection cache. */
static pthread_key_t dmnsn_prtree_caches;
/** Initialize the thread-specific pointer exactly once. */
static pthread_once_t dmnsn_prtree_caches_once = PTHREAD_ONCE_INIT;

/** Needed because pthreads doesn't destroy data from the main thread unless
    it exits with pthread_exit(). */
static void __attribute__((destructor))
dmnsn_delete_main_prtree_caches(void)
{
  dmnsn_delete_array(pthread_getspecific(dmnsn_prtree_caches));
  pthread_key_delete(dmnsn_prtree_caches);
}

static void
dmnsn_delete_prtree_caches(void *caches)
{
  dmnsn_delete_array(caches);
}

static void
dmnsn_initialize_prtree_caches(void)
{
  if (pthread_key_create(&dmnsn_prtree_caches, dmnsn_delete_prtree_caches) != 0)
  {
    dmnsn_error("pthread_key_create() failed.");
  }
}

static dmnsn_intersection_cache *
dmnsn_get_intersection_cache(size_t id)
{
  if (pthread_once(&dmnsn_prtree_caches_once, dmnsn_initialize_prtree_caches)
      != 0)
  {
    dmnsn_error("pthread_once() failed.");
  }

  dmnsn_array *caches = pthread_getspecific(dmnsn_prtree_caches);
  if (!caches) {
    caches = dmnsn_new_array(sizeof(dmnsn_intersection_cache));
    if (pthread_setspecific(dmnsn_prtree_caches, caches) != 0) {
      dmnsn_error("pthread_setspecific() failed.");
    }
  }

  while (dmnsn_array_size(caches) <= id) {
    dmnsn_array_resize(caches, dmnsn_array_size(caches) + 1);
    dmnsn_intersection_cache *cache = dmnsn_array_last(caches);
    cache->i = 0;
    for (size_t i = 0; i < DMNSN_PRTREE_CACHE_SIZE; ++i) {
      cache->objects[i] = NULL;
    }
  }

  return dmnsn_array_at(caches, id);
}

DMNSN_HOT bool
dmnsn_prtree_intersection(const dmnsn_prtree *tree, dmnsn_line ray,
                          dmnsn_intersection *intersection, bool reset)
{
  double t = INFINITY;

  /* Search the unbounded objects */
  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, tree->unbounded) {
    dmnsn_intersection local_intersection;
    if (dmnsn_object_intersection(*object, ray, &local_intersection)) {
      if (local_intersection.t < t) {
        *intersection = local_intersection;
        t = local_intersection.t;
      }
    }
  }

  /* Precalculate 1.0/ray.n.{x,y,z} to save time in intersection tests */
  dmnsn_optimized_line optline = dmnsn_optimize_line(ray);

  /* Search the intersection cache */
  dmnsn_intersection_cache *cache = dmnsn_get_intersection_cache(tree->id);
  if (reset) {
    cache->i = 0;
  }
  dmnsn_object *cached = NULL, *found = NULL;
  if (cache->i < DMNSN_PRTREE_CACHE_SIZE) {
    cached = cache->objects[cache->i];
  }
  if (cached && dmnsn_ray_box_intersection(optline, cached->bounding_box, t)) {
    dmnsn_intersection local_intersection;
    if (dmnsn_object_intersection(cached, ray, &local_intersection)) {
      if (local_intersection.t < t) {
        *intersection = local_intersection;
        t = local_intersection.t;
        found = cached;
      }
    }
  }

  /* Search the bounded objects */
  dmnsn_flat_prnode *node = dmnsn_array_first(tree->bounded);
  while ((size_t)(node - (dmnsn_flat_prnode *)dmnsn_array_first(tree->bounded))
         < dmnsn_array_size(tree->bounded))
  {
    if (dmnsn_ray_box_intersection(optline, node->bounding_box, t)) {
      if (node->object && node->object != cached) {
        dmnsn_intersection local_intersection;
        if (dmnsn_object_intersection(node->object, ray, &local_intersection)) {
          if (local_intersection.t < t) {
            *intersection = local_intersection;
            t = local_intersection.t;
            found = node->object;
          }
        }
      }

      ++node;
    } else {
      node += node->skip;
    }
  }

  /* Update the cache */
  if (cache->i < DMNSN_PRTREE_CACHE_SIZE) {
    cache->objects[cache->i++] = found;
  }

  return !isinf(t);
}

DMNSN_HOT bool
dmnsn_prtree_inside(const dmnsn_prtree *tree, dmnsn_vector point)
{
  /* Search the unbounded objects */
  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, tree->unbounded) {
    if (dmnsn_object_inside(*object, point))
      return true;
  }

  /* Search the bounded objects */
  dmnsn_flat_prnode *node = dmnsn_array_first(tree->bounded);
  while ((size_t)(node - (dmnsn_flat_prnode *)dmnsn_array_first(tree->bounded))
         < dmnsn_array_size(tree->bounded))
  {
    if (dmnsn_bounding_box_contains(node->bounding_box, point)) {
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
