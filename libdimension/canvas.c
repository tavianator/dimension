/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of Dimension.                                       *
 *                                                                       *
 * Dimension is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU Lesser General Public License as published *
 * by the Free Software Foundation; either version 3 of the License, or  *
 * (at your option) any later version.                                   *
 *                                                                       *
 * Dimension is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include "dimension.h"
#include <stdlib.h> /* For malloc(), free() */

dmnsn_canvas *
dmnsn_new_canvas(unsigned int x, unsigned int y)
{
  dmnsn_canvas *canvas = malloc(sizeof(dmnsn_canvas));

  if (canvas) {
    canvas->x = x;
    canvas->y = y;
    canvas->pixels = malloc(sizeof(dmnsn_color)*x*y);

    if (canvas->pixels) {
      return canvas;
    } else {
      free(canvas);
      return NULL;
    }
  } else {
    return NULL;
  }
}

void
dmnsn_delete_canvas(dmnsn_canvas *canvas)
{
  if (canvas) {
    free(canvas->pixels);
    free(canvas);
  }
}
