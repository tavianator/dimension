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
#include <math.h>

/*
 * Finish combinations
 */

static dmnsn_color
dmnsn_finish_combination_fn(const dmnsn_finish *finish,
                            dmnsn_color light, dmnsn_color color,
                            dmnsn_vector ray, dmnsn_vector normal,
                            dmnsn_vector viewer)
{
  dmnsn_finish **params = finish->ptr;
  if (params[0]->finish_fn && params[1]->finish_fn) {
    return dmnsn_color_add((*params[0]->finish_fn)(params[0], light, color, ray,
                                                   normal, viewer),
                           (*params[1]->finish_fn)(params[1], light, color, ray,
                                                   normal, viewer));
  } else if (params[0]->finish_fn) {
    return (*params[0]->finish_fn)(params[0], light, color, ray,
                                   normal, viewer);
  } else if (params[1]->finish_fn) {
    return (*params[1]->finish_fn)(params[1], light, color, ray,
                                   normal, viewer);
  } else {
    return dmnsn_black;
  }
}

static dmnsn_color
dmnsn_finish_combination_ambient_fn(const dmnsn_finish *finish,
                                    dmnsn_color pigment)
{
  dmnsn_finish **params = finish->ptr;
  if (params[0]->ambient_fn && params[1]->ambient_fn) {
    return dmnsn_color_add((*params[0]->ambient_fn)(params[0], pigment),
                           (*params[1]->ambient_fn)(params[1], pigment));
  } else if (params[0]->ambient_fn) {
    return (*params[0]->ambient_fn)(params[0], pigment);
  } else if (params[1]->ambient_fn) {
    return (*params[1]->ambient_fn)(params[1], pigment);
  } else {
    return dmnsn_black;
  }
}

static void
dmnsn_finish_combination_free_fn(void *ptr)
{
  dmnsn_finish **params = ptr;
  dmnsn_delete_finish(params[0]);
  dmnsn_delete_finish(params[1]);
  free(ptr);
}

dmnsn_finish *
dmnsn_new_finish_combination(dmnsn_finish *f1, dmnsn_finish *f2)
{
  if (f1 && f2) {
    dmnsn_finish *finish = dmnsn_new_finish();
    if (finish) {
      dmnsn_finish **params = malloc(2*sizeof(dmnsn_finish *));
      if (!params) {
        dmnsn_delete_finish(finish);
        dmnsn_delete_finish(f2);
        dmnsn_delete_finish(f1);
        return NULL;
      }

      params[0] = f1;
      params[1] = f2;

      finish->ptr        = params;
      finish->finish_fn  = &dmnsn_finish_combination_fn;
      finish->ambient_fn = &dmnsn_finish_combination_ambient_fn;
      finish->free_fn    = &dmnsn_finish_combination_free_fn;

      return finish;
    } else {
      dmnsn_delete_finish(f2);
      dmnsn_delete_finish(f1);
    }
  } else if (f1) {
    dmnsn_delete_finish(f1);
  } else if (f2) {
    dmnsn_delete_finish(f2);
  }

  return NULL;
}
