/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Rendering.
 */

#ifndef DMNSN_RENDER_H
#error "Please include <dimension/render.h> instead of this header directly."
#endif

/**
 * Render a scene.
 * @param[in,out] scene  The scene to render.
 */
void dmnsn_render(dmnsn_scene *scene);

/**
 * Render a scene in the background.
 * @param[in,out] scene  The scene to render.
 * @return A \p dmnsn_future object.
 */
dmnsn_future *dmnsn_render_async(dmnsn_scene *scene);
