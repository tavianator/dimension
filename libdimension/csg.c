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

#include "dimension-impl.h"
#include <stdlib.h>

/* Apply the properties of `csg' to its children */
static void
dmnsn_csg_cascade(const dmnsn_object *csg, dmnsn_object *object)
{
  if (!object->texture && csg->texture) {
    object->texture = csg->texture;
    ++*object->texture->refcount;
  }

  if (!object->interior && csg->interior) {
    object->interior = csg->interior;
    ++*object->interior->refcount;
  }

  object->trans = dmnsn_matrix_mul(csg->trans, object->trans);
}

/* Unions */

static bool
dmnsn_csg_union_intersection_fn(const dmnsn_object *csg,
                                dmnsn_line line,
                                dmnsn_intersection *intersection)
{
  dmnsn_prtree *prtree = csg->ptr;
  return dmnsn_prtree_intersection(prtree, line, intersection);
}

static bool
dmnsn_csg_union_inside_fn(const dmnsn_object *csg, dmnsn_vector point)
{
  dmnsn_prtree *prtree = csg->ptr;
  return dmnsn_prtree_inside(prtree, point);
}

static void
dmnsn_csg_union_init_fn(dmnsn_object *csg)
{
  DMNSN_ARRAY_FOREACH (dmnsn_object **, child, csg->children) {
    dmnsn_csg_cascade(csg, *child);
    dmnsn_object_init(*child);
  }
  csg->trans = dmnsn_identity_matrix();

  dmnsn_prtree *prtree = dmnsn_new_prtree(csg->children);
  csg->ptr = prtree;
  csg->bounding_box = prtree->bounding_box;
}

static void
dmnsn_csg_union_free_fn(void *ptr)
{
  dmnsn_delete_prtree(ptr);
}

/* Bulk-load a union */
dmnsn_object *
dmnsn_new_csg_union(dmnsn_array *objects)
{
  dmnsn_object *csg = dmnsn_new_object();

  DMNSN_ARRAY_FOREACH (dmnsn_object **, object, objects) {
    dmnsn_array_push(csg->children, object);
  }

  csg->intersection_fn = &dmnsn_csg_union_intersection_fn;
  csg->inside_fn       = &dmnsn_csg_union_inside_fn;
  csg->init_fn         = &dmnsn_csg_union_init_fn;
  csg->free_fn         = &dmnsn_csg_union_free_fn;

  return csg;
}

/* Generic CSG free function */
static void
dmnsn_csg_free_fn(void *ptr)
{
  dmnsn_object **params = ptr;
  dmnsn_delete_object(params[1]);
  dmnsn_delete_object(params[0]);
  free(ptr);
}

/* Generic CSG intersection function */
static bool
dmnsn_csg_intersection_fn(const dmnsn_object *csg, dmnsn_line line,
                          dmnsn_intersection *intersection,
                          bool inside1, bool inside2)
{
  /* inside1 is whether the second object is allowed inside the first object;
     respectively for inside2 */

  const dmnsn_object **params = csg->ptr;

  dmnsn_intersection i1, i2;
  bool is_i1 = (*params[0]->intersection_fn)(params[0], line, &i1);
  bool is_i2 = (*params[1]->intersection_fn)(params[1], line, &i2);

  double oldt = 0.0;
  while (is_i1) {
    i1.ray = line;
    i1.t += oldt;
    oldt = i1.t + dmnsn_epsilon;

    dmnsn_vector point = dmnsn_line_point(i1.ray, i1.t);
    if (inside2 ^ (*params[1]->inside_fn)(params[1], point)) {
      dmnsn_line newline = line;
      newline.x0 = dmnsn_line_point(line, i1.t);
      newline    = dmnsn_line_add_epsilon(newline);
      is_i1 = (*params[0]->intersection_fn)(params[0], newline, &i1);
    } else {
      break;
    }
  }

  oldt = 0.0;
  while (is_i2) {
    i2.ray = line;
    i2.t += oldt;
    oldt = i2.t + dmnsn_epsilon;

    dmnsn_vector point = dmnsn_line_point(i2.ray, i2.t);
    if (inside1 ^ (*params[0]->inside_fn)(params[0], point)) {
      dmnsn_line newline = line;
      newline.x0 = dmnsn_line_point(line, i2.t);
      newline    = dmnsn_line_add_epsilon(newline);
      is_i2 = (*params[1]->intersection_fn)(params[1], newline, &i2);
    } else {
      break;
    }
  }

  if (is_i1 && is_i2) {
    if (i1.t < i2.t) {
      *intersection = i1;
    } else {
      *intersection = i2;
    }
  } else if (is_i1) {
    *intersection = i1;
  } else if (is_i2) {
    *intersection = i2;
  } else {
    return false;
  }

  return true;
}

/* Intersections */

static bool
dmnsn_csg_intersection_intersection_fn(const dmnsn_object *csg,
                                       dmnsn_line line,
                                       dmnsn_intersection *intersection)
{
  return dmnsn_csg_intersection_fn(csg, line, intersection, true, true);
}

static bool
dmnsn_csg_intersection_inside_fn(const dmnsn_object *csg, dmnsn_vector point)
{
  dmnsn_object **params = csg->ptr;
  return (*params[0]->inside_fn)(params[0], point)
      && (*params[1]->inside_fn)(params[1], point);
}

static void
dmnsn_csg_intersection_init_fn(dmnsn_object *csg)
{
  dmnsn_object **params = csg->ptr;
  dmnsn_object *A = params[0];
  dmnsn_object *B = params[1];

  dmnsn_csg_cascade(csg, A);
  dmnsn_csg_cascade(csg, B);

  dmnsn_object_init(A);
  dmnsn_object_init(B);

  csg->trans = dmnsn_identity_matrix();
  csg->bounding_box.min
    = dmnsn_vector_max(A->bounding_box.min, B->bounding_box.min);
  csg->bounding_box.max
    = dmnsn_vector_min(A->bounding_box.max, B->bounding_box.max);
}

dmnsn_object *
dmnsn_new_csg_intersection(dmnsn_object *A, dmnsn_object *B)
{
  dmnsn_object *csg = dmnsn_new_object();

  dmnsn_object **params = dmnsn_malloc(2*sizeof(dmnsn_object *));
  params[0] = A;
  params[1] = B;

  csg->ptr             = params;
  csg->intersection_fn = &dmnsn_csg_intersection_intersection_fn;
  csg->inside_fn       = &dmnsn_csg_intersection_inside_fn;
  csg->init_fn         = &dmnsn_csg_intersection_init_fn;
  csg->free_fn         = &dmnsn_csg_free_fn;

  return csg;
}

/* Differences */

static bool
dmnsn_csg_difference_intersection_fn(const dmnsn_object *csg,
                                     dmnsn_line line,
                                     dmnsn_intersection *intersection)
{
  return dmnsn_csg_intersection_fn(csg, line, intersection, true, false);
}

static bool
dmnsn_csg_difference_inside_fn(const dmnsn_object *csg, dmnsn_vector point)
{
  dmnsn_object **params = csg->ptr;
  return (*params[0]->inside_fn)(params[0], point)
     && !(*params[1]->inside_fn)(params[1], point);
}

static void
dmnsn_csg_difference_init_fn(dmnsn_object *csg)
{
  dmnsn_object **params = csg->ptr;
  dmnsn_object *A = params[0];
  dmnsn_object *B = params[1];

  dmnsn_csg_cascade(csg, A);
  dmnsn_csg_cascade(csg, B);

  dmnsn_object_init(A);
  dmnsn_object_init(B);

  csg->trans = dmnsn_identity_matrix();
  csg->bounding_box = A->bounding_box;
}

dmnsn_object *
dmnsn_new_csg_difference(dmnsn_object *A, dmnsn_object *B)
{
  dmnsn_object *csg = dmnsn_new_object();

  dmnsn_object **params = dmnsn_malloc(2*sizeof(dmnsn_object *));
  params[0] = A;
  params[1] = B;

  csg->ptr             = params;
  csg->intersection_fn = &dmnsn_csg_difference_intersection_fn;
  csg->inside_fn       = &dmnsn_csg_difference_inside_fn;
  csg->init_fn         = &dmnsn_csg_difference_init_fn;
  csg->free_fn         = &dmnsn_csg_free_fn;

  return csg;
}

/* Merges */

static bool
dmnsn_csg_merge_intersection_fn(const dmnsn_object *csg,
                                dmnsn_line line,
                                dmnsn_intersection *intersection)
{
  return dmnsn_csg_intersection_fn(csg, line, intersection, false, false);
}

static bool
dmnsn_csg_merge_inside_fn(const dmnsn_object *csg, dmnsn_vector point)
{
  dmnsn_object **params = csg->ptr;
  return (*params[0]->inside_fn)(params[0], point)
      || (*params[1]->inside_fn)(params[1], point);
}

static void
dmnsn_csg_merge_init_fn(dmnsn_object *csg)
{
  dmnsn_object **params = csg->ptr;
  dmnsn_object *A = params[0];
  dmnsn_object *B = params[1];

  dmnsn_csg_cascade(csg, A);
  dmnsn_csg_cascade(csg, B);

  dmnsn_object_init(A);
  dmnsn_object_init(B);

  csg->trans = dmnsn_identity_matrix();
  csg->bounding_box.min
    = dmnsn_vector_min(A->bounding_box.min, B->bounding_box.min);
  csg->bounding_box.max
    = dmnsn_vector_max(A->bounding_box.max, B->bounding_box.max);
}

dmnsn_object *
dmnsn_new_csg_merge(dmnsn_object *A, dmnsn_object *B)
{
  dmnsn_object *csg = dmnsn_new_object();

  dmnsn_object **params = dmnsn_malloc(2*sizeof(dmnsn_object *));
  params[0] = A;
  params[1] = B;

  csg->ptr             = params;
  csg->intersection_fn = &dmnsn_csg_merge_intersection_fn;
  csg->inside_fn       = &dmnsn_csg_merge_inside_fn;
  csg->init_fn         = &dmnsn_csg_merge_init_fn;
  csg->free_fn         = &dmnsn_csg_free_fn;

  return csg;
}
