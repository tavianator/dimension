/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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
#include <stdlib.h>

dmnsn_vector
dmnsn_transform_normal(dmnsn_matrix trans, dmnsn_vector normal)
{
  return dmnsn_vector_normalize(
    dmnsn_vector_sub(
      dmnsn_transform_vector(trans, normal),
      dmnsn_transform_vector(trans, dmnsn_zero)
    )
  );
}

/* Allocate a dummy object */
dmnsn_object *
dmnsn_new_object()
{
  dmnsn_object *object = dmnsn_malloc(sizeof(dmnsn_object));
  object->texture  = NULL;
  object->interior = NULL;
  object->trans    = dmnsn_identity_matrix();
  object->children = dmnsn_new_array(sizeof(dmnsn_object *));
  object->init_fn  = NULL;
  object->free_fn  = NULL;
  return object;
}

/* Free a dummy object */
void
dmnsn_delete_object(dmnsn_object *object)
{
  if (object) {
    DMNSN_ARRAY_FOREACH (dmnsn_object **, child, object->children) {
      dmnsn_delete_object(*child);
    }
    dmnsn_delete_array(object->children);
    dmnsn_delete_interior(object->interior);
    dmnsn_delete_texture(object->texture);
    if (object->free_fn) {
      (*object->free_fn)(object->ptr);
    }
    dmnsn_free(object);
  }
}

/* Precompute object properties */
void
dmnsn_object_init(dmnsn_object *object)
{
  if (object->init_fn) {
    (*object->init_fn)(object);
  }

  object->bounding_box
    = dmnsn_transform_bounding_box(object->trans, object->bounding_box);
  object->trans_inv = dmnsn_matrix_inverse(object->trans);

  if (object->texture) {
    object->texture->trans
      = dmnsn_matrix_mul(object->trans, object->texture->trans);
    dmnsn_texture_init(object->texture);
  }
}
