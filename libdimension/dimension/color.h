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

/*
 * Types to represent color.
 */

#ifndef DIMENSION_COLOR_H
#define DIMENSION_COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* CIE 1931 xyY color. */
typedef struct {
  double x, y, Y; /* x and y are chromaticity coordinates, and Y is luminance,
                     in the CIE 1931 xyZ color space.  We use an unlimited light
                     model, so x,y in [0, 1] and Y >= 0, with 1 = diffuse
                     white */
  double filter, trans; /* Filter transparancy only lets light of this color
                           through; regular transparancy lets all colors
                           through */
} dmnsn_color;

#ifdef __cplusplus
}
#endif

#endif /* DIMENSION_COLOR_H */
