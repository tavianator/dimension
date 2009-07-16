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

/*
 * Objects.
 */

#ifndef DIMENSION_OBJECT_H
#define DIMENSION_OBJECT_H

/* Forward-declare dmnsn_object */
typedef struct dmnsn_object dmnsn_object;

/* Object callback types */
typedef dmnsn_array *dmnsn_object_intersections_fn(const dmnsn_object *object,
                                                   dmnsn_line line);
typedef int dmnsn_object_inside_fn(const dmnsn_object *object,
                                   dmnsn_vector point);

/* dmnsn_object definition */
struct dmnsn_object {
  /* Surface properties */
  dmnsn_texture *texture;

  /* Transformation matrix */
  dmnsn_matrix trans;

  /* Callback functions */
  dmnsn_object_intersections_fn *intersections_fn;
  dmnsn_object_inside_fn        *inside_fn;
  dmnsn_free_fn                 *free_fn;

  /* Generic pointer for object info */
  void *ptr;
};

/* Allocate a dummy object */
dmnsn_object *dmnsn_new_object();

/* Free an object */
void dmnsn_delete_object(dmnsn_object *object);

#endif /* DIMENSION_OBJECT_H */
