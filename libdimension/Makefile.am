###########################################################################
## Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          ##
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

# Make dmnsn_error() backtraces useful
AM_LDFLAGS = -rdynamic

nobase_include_HEADERS = dimension.h                                           \
                         dimension/array.h                                     \
                         dimension/camera.h                                    \
                         dimension/cameras.h                                   \
                         dimension/canvas.h                                    \
                         dimension/color.h                                     \
                         dimension/csg.h                                       \
                         dimension/error.h                                     \
                         dimension/finishes.h                                  \
                         dimension/geometry.h                                  \
                         dimension/gl.h                                        \
                         dimension/inline.h                                    \
                         dimension/interior.h                                  \
                         dimension/light.h                                     \
                         dimension/lights.h                                    \
                         dimension/list.h                                      \
                         dimension/malloc.h                                    \
                         dimension/map.h                                       \
                         dimension/object.h                                    \
                         dimension/objects.h                                   \
                         dimension/pattern.h                                   \
                         dimension/patterns.h                                  \
                         dimension/pigments.h                                  \
                         dimension/png.h                                       \
                         dimension/polynomial.h                                \
                         dimension/progress.h                                  \
                         dimension/raytrace.h                                  \
                         dimension/scene.h                                     \
                         dimension/sky_sphere.h                                \
                         dimension/texture.h                                   \
                         dimension/timer.h

lib_LTLIBRARIES = libdimension.la

libdimension_la_SOURCES = $(nobase_include_HEADERS)                            \
                          ambient.c                                            \
                          camera.c                                             \
                          canvas.c                                             \
                          canvas_pigment.c                                     \
                          checker.c                                            \
                          color.c                                              \
                          color_map.c                                          \
                          cone.c                                               \
                          cube.c                                               \
                          csg.c                                                \
                          diffuse.c                                            \
                          dimension-impl.h                                     \
                          error.c                                              \
                          finish_combination.c                                 \
                          geometry.c                                           \
                          gradient.c                                           \
                          inline.c                                             \
                          interior.c                                           \
                          light.c                                              \
                          list.c                                               \
                          malloc.c                                             \
                          map.c                                                \
                          object.c                                             \
                          pattern.c                                            \
                          perspective.c                                        \
                          phong.c                                              \
                          pigment_map.c                                        \
                          plane.c                                              \
                          platform.c                                           \
                          platform.h                                           \
                          point_light.c                                        \
                          polynomial.c                                         \
                          progress.c                                           \
                          progress-impl.h                                      \
                          prtree.c                                             \
                          prtree.h                                             \
                          raytrace.c                                           \
                          reflective.c                                         \
                          scene.c                                              \
                          sky_sphere.c                                         \
                          solid_pigment.c                                      \
                          sphere.c                                             \
                          texture.c                                            \
                          threads.c                                            \
                          threads.h                                            \
                          timer.c                                              \
                          torus.c
libdimension_la_CFLAGS  = $(AM_CFLAGS) -pthread
libdimension_la_LDFLAGS = -version-info 0:0:0 -no-undefined $(AM_LDFLAGS)
libdimension_la_LIBADD  = -lm

if PNG
libdimension_la_SOURCES += png.c
libdimension_la_LIBADD  += -lpng
else
libdimension_la_SOURCES += png-stubs.c
endif

if GL
libdimension_la_SOURCES += gl.c
libdimension_la_LIBADD  += -lGL
else
libdimension_la_SOURCES += gl-stubs.c
endif

if PGO

noinst_LTLIBRARIES = libdimension-pgo.la
libdimension_pgo_la_SOURCES = $(libdimension_la_SOURCES)
libdimension_pgo_la_LIBADD  = $(libdimension_la_LIBADD) -lgcov
libdimension_pgo_la_CFLAGS  = $(AM_CFLAGS) -pthread -fprofile-generate=$$(readlink -f $(builddir))/pgo

libdimension_la_CFLAGS += -fprofile-correction -fprofile-use=./pgo

all-local: libdimension.la
	@for Plo in .deps/libdimension_la*.Plo; do \
	  [ -f "pgo/.libs/$$(basename $$Plo .Plo).gcda" ] && sed -ri 's|^(libdimension_la-.*).lo: ([^ ])|\1.lo:  pgo/.libs/\1.gcda \2|g' "$$Plo"; \
	  true; \
	done

clean-pgo:
	rm -rf pgo
clean-local: clean-docs clean-pgo
else
clean-local: clean-docs
endif

doc:
	doxygen Doxyfile
clean-docs:
	rm -rf $(top_builddir)/doc/libdimension

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdimension.pc
