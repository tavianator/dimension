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

/*
 * libdimension - a library for photo-realistic 3-D rendering
 */

#ifndef DIMENSION_H
#define DIMENSION_H

#ifdef __cplusplus
/* We've been included from a C++ file; mark everything here as extern "C" */
extern "C" {
#endif

#include <dimension/error.h>
#include <dimension/array.h>
#include <dimension/progress.h>
#include <dimension/geometry.h>
#include <dimension/color.h>
#include <dimension/canvas.h>
#include <dimension/object.h>
#include <dimension/sphere.h>
#include <dimension/cube.h>
#include <dimension/camera.h>
#include <dimension/scene.h>
#include <dimension/raytrace.h>
#include <dimension/png.h>

#ifdef __cplusplus
}
#endif

#endif /* DIMENSION_H */
