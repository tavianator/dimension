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

#include "dimension.h"
#include <errno.h>

static void
dmnsn_csg_free_fn(void *ptr)
{
  dmnsn_object **params = ptr;
  dmnsn_delete_object(params[1]);
  dmnsn_delete_object(params[0]);
  free(ptr);
}

/* Unions */

static bool
dmnsn_csg_union_intersection_fn(const dmnsn_object *csg,
                                dmnsn_line line,
                                dmnsn_intersection *intersection)
{
  const dmnsn_object **params = csg->ptr;

  dmnsn_line line1 = dmnsn_matrix_line_mul(params[0]->trans_inv, line);
  dmnsn_line line2 = dmnsn_matrix_line_mul(params[1]->trans_inv, line);

  dmnsn_intersection i1, i2;
  bool is_i1 = (*params[0]->intersection_fn)(params[0], line1, &i1);
  bool is_i2 = (*params[1]->intersection_fn)(params[1], line2, &i2);

  if (is_i1) {
    /* Transform the intersection back to the observer's view */
    i1.ray = line;
    i1.normal = dmnsn_vector_normalize(
      dmnsn_vector_sub(
        dmnsn_matrix_vector_mul(params[0]->trans, i1.normal),
        dmnsn_matrix_vector_mul(params[0]->trans, dmnsn_zero)
      )
    );

    if (!i1.texture)
      i1.texture = csg->texture;
    if (!i1.interior)
      i1.interior = csg->interior;
  }

  if (is_i2) {
    i2.ray = line;
    i2.normal = dmnsn_vector_normalize(
      dmnsn_vector_sub(
        dmnsn_matrix_vector_mul(params[1]->trans, i2.normal),
        dmnsn_matrix_vector_mul(params[1]->trans, dmnsn_zero)
      )
    );

    if (!i2.texture)
      i2.texture = csg->texture;
    if (!i2.interior)
      i2.interior = csg->interior;
  }

  if (!is_i1 && !is_i2) {
    return false;
  } else if (is_i1 && !is_i2) {
    *intersection = i1;
  } else if (!is_i1 && is_i2) {
    *intersection = i2;
  } else if (i1.t < i2.t) {
    *intersection = i1;
  } else {
    *intersection = i2;
  }
  return true;
}

static bool
dmnsn_csg_union_inside_fn(const dmnsn_object *csg, dmnsn_vector point)
{
  dmnsn_object **params = csg->ptr;
  return (*params[0]->inside_fn)(params[0], point)
      || (*params[1]->inside_fn)(params[1], point);
}

dmnsn_object *
dmnsn_new_csg_union(dmnsn_object *A, dmnsn_object *B)
{
  A->trans_inv = dmnsn_matrix_inverse(A->trans);
  B->trans_inv = dmnsn_matrix_inverse(B->trans);

  dmnsn_object *csg = dmnsn_new_object();

  dmnsn_object **params = dmnsn_malloc(2*sizeof(dmnsn_object *));
  params[0] = A;
  params[1] = B;

  csg->ptr             = params;
  csg->intersection_fn = &dmnsn_csg_union_intersection_fn;
  csg->inside_fn       = &dmnsn_csg_union_inside_fn;
  csg->free_fn         = &dmnsn_csg_free_fn;

  dmnsn_bounding_box Abox
    = dmnsn_matrix_bounding_box_mul(A->trans, A->bounding_box);
  dmnsn_bounding_box Bbox
    = dmnsn_matrix_bounding_box_mul(B->trans, B->bounding_box);
  csg->bounding_box.min = dmnsn_vector_min(Abox.min, Bbox.min);
  csg->bounding_box.max = dmnsn_vector_max(Abox.max, Bbox.max);

  return csg;
}

/* Generic CSG intersection function */
static bool
dmnsn_csg_intersection_fn(const dmnsn_object *csg, dmnsn_line line,
                          dmnsn_intersection *intersection,
                          bool inside1, bool inside2)
{
  /* inside1 is whether the first object is allowed inside the second object;
     respectively for inside2 */

  const dmnsn_object **params = csg->ptr;

  dmnsn_line line1 = dmnsn_matrix_line_mul(params[0]->trans_inv, line);
  dmnsn_line line2 = dmnsn_matrix_line_mul(params[1]->trans_inv, line);

  dmnsn_intersection i1, i2;
  bool is_i1 = (*params[0]->intersection_fn)(params[0], line1, &i1);
  bool is_i2 = (*params[1]->intersection_fn)(params[1], line2, &i2);

  double oldt = 0.0;
  while (is_i1) {
    i1.ray = line;
    i1.t += oldt;
    oldt = i1.t + dmnsn_epsilon;
    i1.normal = dmnsn_vector_normalize(
      dmnsn_vector_sub(
        dmnsn_matrix_vector_mul(params[0]->trans, i1.normal),
        dmnsn_matrix_vector_mul(params[0]->trans, dmnsn_zero)
      )
    );

    if (!i1.texture)
      i1.texture = csg->texture;
    if (!i1.interior)
      i1.interior = csg->interior;

    dmnsn_vector point = dmnsn_line_point(i1.ray, i1.t);
    point = dmnsn_matrix_vector_mul(params[1]->trans_inv, point);
    if (inside1 ^ (*params[1]->inside_fn)(params[1], point)) {
      line1.x0 = dmnsn_line_point(line1, i1.t);
      line1 = dmnsn_line_add_epsilon(line1);
      is_i1 = (*params[0]->intersection_fn)(params[0], line1, &i1);
    } else {
      break;
    }
  }

  oldt = 0.0;
  while (is_i2) {
    i2.ray = line;
    i2.t += oldt;
    oldt = i2.t + dmnsn_epsilon;
    i2.normal = dmnsn_vector_normalize(
      dmnsn_vector_sub(
        dmnsn_matrix_vector_mul(params[1]->trans, i2.normal),
        dmnsn_matrix_vector_mul(params[1]->trans, dmnsn_zero)
      )
    );

    if (!i2.texture)
      i2.texture = csg->texture;
    if (!i2.interior)
      i2.interior = csg->interior;

    dmnsn_vector point = dmnsn_line_point(i2.ray, i2.t);
    point = dmnsn_matrix_vector_mul(params[0]->trans_inv, point);
    if (inside2 ^ (*params[0]->inside_fn)(params[0], point)) {
      line2.x0 = dmnsn_line_point(line2, i2.t);
      line2 = dmnsn_line_add_epsilon(line2);
      is_i2 = (*params[1]->intersection_fn)(params[1], line2, &i2);
    } else {
      break;
    }
  }

  if (!is_i1 && !is_i2) {
    return false;
  } else if (is_i1 && !is_i2) {
    *intersection = i1;
  } else if (!is_i1 && is_i2) {
    *intersection = i2;
  } else if (i1.t < i2.t) {
    *intersection = i1;
  } else {
    *intersection = i2;
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

dmnsn_object *
dmnsn_new_csg_intersection(dmnsn_object *A, dmnsn_object *B)
{
  A->trans_inv = dmnsn_matrix_inverse(A->trans);
  B->trans_inv = dmnsn_matrix_inverse(B->trans);

  dmnsn_object *csg = dmnsn_new_object();

  dmnsn_object **params = dmnsn_malloc(2*sizeof(dmnsn_object *));
  params[0] = A;
  params[1] = B;

  csg->ptr             = params;
  csg->intersection_fn = &dmnsn_csg_intersection_intersection_fn;
  csg->inside_fn       = &dmnsn_csg_intersection_inside_fn;
  csg->free_fn         = &dmnsn_csg_free_fn;

  dmnsn_bounding_box Abox
    = dmnsn_matrix_bounding_box_mul(A->trans, A->bounding_box);
  dmnsn_bounding_box Bbox
    = dmnsn_matrix_bounding_box_mul(B->trans, B->bounding_box);
  csg->bounding_box.min = dmnsn_vector_max(Abox.min, Bbox.min);
  csg->bounding_box.max = dmnsn_vector_min(Abox.max, Bbox.max);

  return csg;
}

/* Differences */

static bool
dmnsn_csg_difference_intersection_fn(const dmnsn_object *csg,
                                     dmnsn_line line,
                                     dmnsn_intersection *intersection)
{
  return dmnsn_csg_intersection_fn(csg, line, intersection, false, true);
}

static bool
dmnsn_csg_difference_inside_fn(const dmnsn_object *csg, dmnsn_vector point)
{
  dmnsn_object **params = csg->ptr;
  return (*params[0]->inside_fn)(params[0], point)
     && !(*params[1]->inside_fn)(params[1], point);
}

dmnsn_object *
dmnsn_new_csg_difference(dmnsn_object *A, dmnsn_object *B)
{
  A->trans_inv = dmnsn_matrix_inverse(A->trans);
  B->trans_inv = dmnsn_matrix_inverse(B->trans);

  dmnsn_object *csg = dmnsn_new_object();

  dmnsn_object **params = dmnsn_malloc(2*sizeof(dmnsn_object *));
  params[0] = A;
  params[1] = B;

  csg->ptr             = params;
  csg->intersection_fn = &dmnsn_csg_difference_intersection_fn;
  csg->inside_fn       = &dmnsn_csg_difference_inside_fn;
  csg->free_fn         = &dmnsn_csg_free_fn;
  csg->bounding_box = dmnsn_matrix_bounding_box_mul(A->trans, A->bounding_box);

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

dmnsn_object *
dmnsn_new_csg_merge(dmnsn_object *A, dmnsn_object *B)
{
  A->trans_inv = dmnsn_matrix_inverse(A->trans);
  B->trans_inv = dmnsn_matrix_inverse(B->trans);

  dmnsn_object *csg = dmnsn_new_object();

  dmnsn_object **params = dmnsn_malloc(2*sizeof(dmnsn_object *));
  params[0] = A;
  params[1] = B;

  csg->ptr             = params;
  csg->intersection_fn = &dmnsn_csg_merge_intersection_fn;
  csg->inside_fn       = &dmnsn_csg_merge_inside_fn;
  csg->free_fn         = &dmnsn_csg_free_fn;

  dmnsn_bounding_box Abox
    = dmnsn_matrix_bounding_box_mul(A->trans, A->bounding_box);
  dmnsn_bounding_box Bbox
    = dmnsn_matrix_bounding_box_mul(B->trans, B->bounding_box);
  csg->bounding_box.min = dmnsn_vector_min(Abox.min, Bbox.min);
  csg->bounding_box.max = dmnsn_vector_max(Abox.max, Bbox.max);

  return csg;
}
