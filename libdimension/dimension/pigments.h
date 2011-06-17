/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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

/**
 * Construct a color map.
 * @return An empty color map.
 */
dmnsn_map *dmnsn_new_color_map(void);

/**
 * pigment_map flags.
 */
typedef enum dmnsn_pigment_map_flags {
  DMNSN_PIGMENT_MAP_REGULAR, /**< Calculate linear color gradients. */
  DMNSN_PIGMENT_MAP_SRGB     /**< Calculate sRGB color gradients. */
} dmnsn_pigment_map_flags;

/**
 * A color-mapped pigment.
 * @param[in,out] pattern  The pattern of the pigment.
 * @param[in,out] map      The color map to apply to the pattern.
 * @param[in]     flags    Gradient flags
 * @return A pigment mapping the pattern to color values.
 */
dmnsn_pigment *dmnsn_new_color_map_pigment(dmnsn_pattern *pattern,
                                           dmnsn_map *map,
                                           dmnsn_pigment_map_flags flags);

/**
 * Construct a pigment map.
 * @return An empty pigment map.
 */
dmnsn_map *dmnsn_new_pigment_map(void);

/**
 * A pigment-mapped pigment.
 * @param[in,out] pattern  The pattern of the pigment.
 * @param[in,out] map      The pigment map to apply to the pattern.
 * @param[in]     flags    Gradient flags
 * @return A pigment mapping the pattern to other pigments.
 */
dmnsn_pigment *dmnsn_new_pigment_map_pigment(dmnsn_pattern *pattern,
                                             dmnsn_map *map,
                                             dmnsn_pigment_map_flags flags);
