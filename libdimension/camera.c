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

/* Allocate a new dummy camera */
dmnsn_camera *
dmnsn_new_camera()
{
  dmnsn_camera *camera = dmnsn_malloc(sizeof(dmnsn_camera));
  camera->free_fn = NULL;
  return camera;
}

/* Free a dummy camera */
void
dmnsn_delete_camera(dmnsn_camera *camera)
{
  if (camera) {
    if (camera->free_fn) {
      (*camera->free_fn)(camera->ptr);
    }
    dmnsn_free(camera);
  }
}
