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

/**
 * @file
 * Pre-defined pigments.
 */

#ifndef DIMENSION_PIGMENTS_H
#define DIMENSION_PIGMENTS_H

/**
 * A solid color.
 * @param[in] color  The color of the pigment.
 * @return A pigment with the color \p color everywhere.
 */
dmnsn_pigment *dmnsn_new_solid_pigment(dmnsn_color color);

/**
 * An image map.  The image (regardless of its real dimensions) is projected
 * on the x-y plane in tesselating unit squares.
 * @param[in] canvas  The canvas holding the image.
 * @return An image-mapped pigment.
 */
dmnsn_pigment *dmnsn_new_canvas_pigment(dmnsn_canvas *canvas);

/** Color map. */
typedef dmnsn_array dmnsn_color_map;

/**
 * Create an empty color map.
 * @return A color map with no entries.
 */
dmnsn_color_map *dmnsn_new_color_map(void);

/**
 * Delete a color map.
 * @param[in,out] map  The color map to delete.
 */
void dmnsn_delete_color_map(dmnsn_color_map *map);

/**
 * Add an entry (a scalar-color pair) to a color map.
 * @param[in,out] map  The color map to add to.
 * @param[in]     n    The index of the entry.
 * @param[in]     c    The value of the entry.
 */
void dmnsn_add_color_map_entry(dmnsn_color_map *map, double n, dmnsn_color c);

/**
 * Evaluate a color map.
 * @param[in] map  The map to evaluate.
 * @param[in] n    The index to evaluate.
 * @return The value of the gradient between the the two indicies closest to
 * \p n.
 */
dmnsn_color dmnsn_color_map_value(const dmnsn_color_map *map, double n);

/**
 * A color-mapped pigment.
 * @param[in,out] pattern  The pattern of the pigment.
 * @param[in,out] map      The color map to apply to the pattern.
 * @return A pigment mapping the pattern to color values.
 */
dmnsn_pigment *dmnsn_new_color_map_pigment(dmnsn_pattern *pattern,
                                           dmnsn_color_map *map);

#endif /* DIMENSION_PIGMENTS_H */
