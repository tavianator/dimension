/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * The main \#include file for libdimension.  This file declares all public
 * functions and types used by the Dimension library.  You should never attempt
 * to include any of the component headers in the dimension/ subdirectory
 * directly; only this file.
 */

/**
 * @mainpage libdimension - A library for photo-realistic 3-D rendering
 *
 * The Dimension library (libdimension) is the ray-tracing library that handles
 * all rendering-related tasks for Dimension.  It is written in C and designed
 * with performance and concurrency in mind.  It is also generic enough to be
 * used for applications other than Dimension, though that is its primary
 * purpose.
 */

#ifndef DIMENSION_H
#define DIMENSION_H

#ifdef __cplusplus
/* We've been included from a C++ file; mark everything here as extern "C" */
extern "C" {
#endif

/* Common macros */

/**
 * @internal
 * @def DMNSN_FUNC
 * @brief Expands to the name of the current function
 */
#ifdef __GNUC__
  #define DMNSN_FUNC __PRETTY_FUNCTION__
#elif __STDC_VERSION__ >= 199901L
  #define DMNSN_FUNC __func__
#else
  #define DMNSN_FUNC "<unknown function>"
#endif

/* Common types */

/**
 * Generic callback type.
 * @param[in,out] ptr  A pointer to an object to act on.
 */
typedef void dmnsn_callback_fn(void *ptr);

/**
 * Destructor callback type.
 * @param[in,out] ptr  The pointer to free.
 */
typedef void dmnsn_free_fn(void *ptr);

/* Include all the libdimension headers */
#include <dimension/inline.h>
#include <dimension/error.h>
#include <dimension/malloc.h>
#include <dimension/refcount.h>
#include <dimension/array.h>
#include <dimension/dictionary.h>
#include <dimension/future.h>
#include <dimension/timer.h>
#include <dimension/geometry.h>
#include <dimension/polynomial.h>
#include <dimension/color.h>
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
#include <dimension/raytrace.h>

#ifdef __cplusplus
}
#endif

#endif /* DIMENSION_H */
