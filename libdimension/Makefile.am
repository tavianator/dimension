###########################################################################
## Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     ##
##                                                                       ##
## This file is part of The Dimension Build Suite.                       ##
##                                                                       ##
## The Dimension Build Suite is free software; you can redistribute it   ##
## and/or modify it under the terms of the GNU General Public License as ##
## published by the Free Software Foundation; either version 3 of the    ##
## License, or (at your option) any later version.                       ##
##                                                                       ##
## The Dimension Build Suite is distributed in the hope that it will be  ##
## useful, but WITHOUT ANY WARRANTY; without even the implied warranty   ##
## of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  ##
## General Public License for more details.                              ##
##                                                                       ##
## You should have received a copy of the GNU General Public License     ##
## along with this program.  If not, see <http://www.gnu.org/licenses/>. ##
###########################################################################

SUBDIRS = . \
  bench \
  tests

AM_CFLAGS = $(PTHREAD_CFLAGS)
# Make dmnsn_error() backtraces useful
AM_LDFLAGS = $(PTHREAD_LIBS) -rdynamic
CC = $(PTHREAD_CC)

nobase_include_HEADERS = dimension.h \
  dimension/base.h \
  dimension/base/compiler.h \
  dimension/base/common.h \
  dimension/base/error.h \
  dimension/base/malloc.h \
  dimension/base/pool.h \
  dimension/base/array.h \
  dimension/base/dictionary.h \
  dimension/concurrency.h \
  dimension/concurrency/future.h \
  dimension/platform.h \
  dimension/platform/timer.h \
  dimension/math.h \
  dimension/math/aabb.h \
  dimension/math/matrix.h \
  dimension/math/ray.h \
  dimension/math/scalar.h \
  dimension/math/vector.h \
  dimension/color.h \
  dimension/color/color.h \
  dimension/color/tcolor.h \
  dimension/canvas.h \
  dimension/canvas/canvas.h \
  dimension/canvas/gl.h \
  dimension/canvas/png.h \
  dimension/pattern.h \
  dimension/pattern/pattern.h \
  dimension/pattern/patterns.h \
  dimension/pattern/map.h \
  dimension/model.h \
  dimension/model/camera.h \
  dimension/model/cameras.h \
  dimension/model/csg.h \
  dimension/model/finishes.h \
  dimension/model/finish.h \
  dimension/model/interior.h \
  dimension/model/light.h \
  dimension/model/lights.h \
  dimension/model/object.h \
  dimension/model/objects.h \
  dimension/model/pigment.h \
  dimension/model/pigments.h \
  dimension/model/scene.h \
  dimension/model/texture.h \
  dimension/render.h \
  dimension/render/render.h

lib_LTLIBRARIES = libdimension.la

libdimension_la_SOURCES = $(nobase_include_HEADERS) \
  base/array.c \
  base/dictionary.c \
  base/error.c \
  base/inline.c \
  base/malloc.c \
  base/pool.c \
  bvh/bvh.c \
  bvh/prtree.c \
  canvas/canvas.c \
  canvas/rgba.c \
  concurrency/future.c \
  concurrency/threads.c \
  dimension.h \
  internal.h \
  internal/all.h \
  internal/bvh.h \
  internal/compiler.h \
  internal/future.h \
  internal/platform.h \
  internal/polynomial.h \
  internal/profile.h \
  internal/prtree.h \
  internal/rgba.h \
  internal/threads.h \
  math/matrix.c \
  math/polynomial.c \
  model/camera.c \
  model/cameras/perspective.c \
  model/finish.c \
  model/finishes/lambertian.c \
  model/finishes/phong.c \
  model/finishes/reflection.c \
  model/interior.c \
  model/light.c \
  model/lights/point_light.c \
  model/object.c \
  model/objects/cone.c \
  model/objects/csg.c \
  model/objects/cube.c \
  model/objects/plane.c \
  model/objects/sphere.c \
  model/objects/torus.c \
  model/objects/triangle.c \
  model/objects/triangle_fan.c \
  model/pigment.c \
  model/pigments/canvas_pigment.c \
  model/pigments/pigment_map.c \
  model/pigments/solid_pigment.c \
  model/scene.c \
  model/texture.c \
  pattern/checker.c \
  pattern/gradient.c \
  pattern/leopard.c \
  pattern/map.c \
  pattern/pattern.c \
  platform/platform.c \
  platform/timer.c \
  render/render.c
libdimension_la_CFLAGS  = $(AM_CFLAGS)
libdimension_la_LDFLAGS = -version-info 0:0:0 -no-undefined $(AM_LDFLAGS)
libdimension_la_LIBADD  =

if PNG
libdimension_la_SOURCES += canvas/png.c
libdimension_la_CFLAGS  += $(libpng_CFLAGS)
libdimension_la_LIBADD  += $(libpng_LIBS)
else
libdimension_la_SOURCES += canvas/png-stubs.c
endif

if GL
libdimension_la_SOURCES += canvas/gl.c
libdimension_la_LIBADD  += -lGL
else
libdimension_la_SOURCES += canvas/gl-stubs.c
endif

if PROFILE
libdimension_la_SOURCES += base/profile.c
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdimension.pc

bench: all-recursive
	cd bench && $(MAKE) $(AM_MAKEFLAGS) bench

clean-local: clean-doc

doc:
	doxygen Doxyfile
clean-doc:
	rm -rf doc

.PHONY: bench doc
