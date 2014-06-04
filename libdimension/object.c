/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Objects.
 */

#include "dimension-internal.h"
#include <stdlib.h>

/* Allocate a dummy object */
dmnsn_object *
dmnsn_new_object(dmnsn_pool *pool)
{
  dmnsn_object *object = DMNSN_PALLOC(pool, dmnsn_object);
  dmnsn_init_object(pool, object);
  return object;
}

/* Initialize a dmnsn_object field */
void
dmnsn_init_object(dmnsn_pool *pool, dmnsn_object *object)
{
  object->texture         = NULL;
  object->interior        = NULL;
  object->trans           = dmnsn_identity_matrix();
  object->intrinsic_trans = dmnsn_identity_matrix();
  object->children        = NULL;
  object->split_children  = false;
  object->intersection_fn = NULL;
  object->inside_fn       = NULL;
  object->initialize_fn   = NULL;
  object->initialized     = false;
}

/** Recursively initialize objects. */
static void
dmnsn_object_initialize_recursive(dmnsn_object *object,
                                  dmnsn_matrix pigment_trans)
{
  dmnsn_assert(!object->initialized, "Object double-initialized.");
  object->initialized = true;

  /* Initialize the texture */
  if (!object->texture->initialized) {
    dmnsn_texture_initialize(object->texture);
  }

  /* Precalculate object values */
  object->pigment_trans = pigment_trans;
  object->trans = dmnsn_matrix_mul(object->trans, object->intrinsic_trans);

  /* Initialize the object's children */
  if (object->children) {
    DMNSN_ARRAY_FOREACH (dmnsn_object **, child, object->children) {
      (*child)->trans = dmnsn_matrix_mul(object->trans, (*child)->trans);

      dmnsn_matrix child_pigment_trans;
      if ((*child)->texture == NULL || (*child)->texture->pigment == NULL) {
        /* Don't transform cascaded pigments with the child object */
        child_pigment_trans = pigment_trans;
      } else {
        child_pigment_trans = dmnsn_matrix_inverse((*child)->trans);
      }

      dmnsn_texture_cascade(object->texture, &(*child)->texture);
      dmnsn_interior_cascade(object->interior, &(*child)->interior);
      dmnsn_object_initialize_recursive(*child, child_pigment_trans);
    }
  }

  /* Initialization callback */
  if (object->initialize_fn) {
    object->initialize_fn(object);
  }

  /* Precalculate more object values */
  object->bounding_box
    = dmnsn_transform_bounding_box(object->trans, object->bounding_box);
  object->trans_inv = dmnsn_matrix_inverse(object->trans);
}

/* Precompute object properties */
void
dmnsn_object_initialize(dmnsn_object *object)
{
  dmnsn_matrix pigment_trans = dmnsn_matrix_inverse(object->trans);
  dmnsn_object_initialize_recursive(object, pigment_trans);
}
