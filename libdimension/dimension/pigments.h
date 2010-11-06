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
 * Custom pigments.
 */

#ifndef DIMENSION_PIGMENTS_H
#define DIMENSION_PIGMENTS_H

/* A solid color */
dmnsn_pigment *dmnsn_new_solid_pigment(dmnsn_color color);
/* An image map */
dmnsn_pigment *dmnsn_new_canvas_pigment(dmnsn_canvas *canvas);

/* Color maps */
typedef dmnsn_array dmnsn_color_map;

dmnsn_color_map *dmnsn_new_color_map();
void dmnsn_delete_color_map(dmnsn_color_map *map);

void dmnsn_add_color_map_entry(dmnsn_color_map *map, double n, dmnsn_color c);
dmnsn_color dmnsn_color_map_value(const dmnsn_color_map *map, double n);

/* Color-mapped pigments */
dmnsn_pigment *dmnsn_new_color_map_pigment(dmnsn_pattern *pattern,
                                           dmnsn_color_map *map);

#endif /* DIMENSION_PIGMENTS_H */
