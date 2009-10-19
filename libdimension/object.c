/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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
#include <stdlib.h> /* For malloc */

/* Allocate an intersection - cannot fail */
dmnsn_intersection *
dmnsn_new_intersection()
{
  dmnsn_intersection *intersection = malloc(sizeof(dmnsn_intersection));
  if (!intersection) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate an intersection object.");
  }
  return intersection;
}

/* Free an intersection */
void
dmnsn_delete_intersection(dmnsn_intersection *intersection)
{
  free(intersection);
}

/* Allocate a dummy object */
dmnsn_object *
dmnsn_new_object()
{
  dmnsn_object *object = malloc(sizeof(dmnsn_object));
  if (object) {
    object->texture = NULL;
    object->trans   = dmnsn_identity_matrix();
    object->free_fn = NULL;
  }
  return object;
}

/* Free a dummy object */
void
dmnsn_delete_object(dmnsn_object *object)
{
  if (object) {
    dmnsn_delete_texture(object->texture);
    if (object->free_fn) {
      (*object->free_fn)(object->ptr);
    }
    free(object);
  }
}
