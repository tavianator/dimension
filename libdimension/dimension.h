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
 * The main \#include file for the Dimension library.  This file declares all
 * of its public functions and types.
 */

/**
 * @mainpage libdimension - A library for photo-realistic 3-D rendering
 *
 * The Dimension library (libdimension) is the ray-tracing library that handles
 * all rendering-related tasks for Dimension.
 */

#ifndef DIMENSION_H
#define DIMENSION_H

/* Include compiler.h first for DMNSN_CXX */
#include <dimension/compiler.h>

#if DMNSN_CXX
/* We've been included from a C++ file; mark everything here as extern "C" */
extern "C" {
#endif

/* Include all the libdimension headers */
#include <dimension/common.h>
#include <dimension/error.h>
#include <dimension/malloc.h>
#include <dimension/pool.h>
#include <dimension/array.h>
#include <dimension/dictionary.h>
#include <dimension/future.h>
#include <dimension/timer.h>
#include <dimension/math.h>
#include <dimension/geometry.h>
#include <dimension/polynomial.h>
#include <dimension/color.h>
#include <dimension/tcolor.h>
#include <dimension/canvas.h>
#include <dimension/gl.h>
#include <dimension/png.h>
#include <dimension/pattern.h>
#include <dimension/patterns.h>
#include <dimension/map.h>
#include <dimension/pigment.h>
#include <dimension/pigments.h>
#include <dimension/finish.h>
#include <dimension/finishes.h>
#include <dimension/texture.h>
#include <dimension/interior.h>
#include <dimension/object.h>
#include <dimension/objects.h>
#include <dimension/csg.h>
#include <dimension/light.h>
#include <dimension/lights.h>
#include <dimension/camera.h>
#include <dimension/cameras.h>
#include <dimension/scene.h>
#include <dimension/ray_trace.h>

#if DMNSN_CXX
}
#endif

#endif /* DIMENSION_H */
