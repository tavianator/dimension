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

/*
 * Lights.
 */

#ifndef DIMENSION_LIGHT_H
#define DIMENSION_LIGHT_H

typedef struct dmnsn_light dmnsn_light;

typedef dmnsn_color dmnsn_light_fn(const dmnsn_light *light, dmnsn_vector v);

struct dmnsn_light {
  /* Origin of light rays */
  dmnsn_vector x0;

  /* Callbacks */
  dmnsn_light_fn *light_fn;
  dmnsn_free_fn  *free_fn;

  /* Generic pointer for light info */
  void *ptr;
};

dmnsn_light *dmnsn_new_light(void);
void dmnsn_delete_light(dmnsn_light *light);

#endif /* DIMENSION_LIGHT_H */
