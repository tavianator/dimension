/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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

#ifndef DIMENSION_PNG_H
#define DIMENSION_PNG_H

#include <dimension.h>
#include <stdio.h>

#ifdef __cplusplus
/* We've been included from a C++ file; mark everything here as extern "C" */
extern "C" {
#endif

/* Write canvas to file in PNG format.  Returns 0 on success, nonzero on
   failure */
int dmnsn_png_write_canvas(const dmnsn_canvas *canvas, FILE *file);

/* Read a canvas from a PNG file.  Returns NULL on failure. */
dmnsn_canvas *dmnsn_png_read_canvas(FILE *file);

#ifdef __cplusplus
}
#endif

#endif /* DIMENSION_PNG_H */
