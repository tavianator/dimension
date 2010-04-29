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

dmnsn_list *
dmnsn_list_from_array(const dmnsn_array *array)
{
  dmnsn_list *list = dmnsn_new_list(array->obj_size);

  size_t i;
  for (i = 0; i < dmnsn_array_size(array); ++i) {
    dmnsn_list_push(list, dmnsn_array_at(array, i));
  }

  return list;
}

void
dmnsn_delete_list(dmnsn_list *list)
{
  if (list) {
    while (list->first) {
      dmnsn_list_remove(list, list->first);
    }
    free(list);
  }
}
