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

#ifndef DIMENSION_PNG_H
#define DIMENSION_PNG_H

#include <dimension.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int dmnsn_png_write_canvas(const dmnsn_canvas *canvas, FILE *file);
int dmnsn_png_read_canvas(dmnsn_canvas **canvas, FILE *file);

#ifdef __cplusplus
}
#endif

#endif /* DIMENSION_PNG_H */
