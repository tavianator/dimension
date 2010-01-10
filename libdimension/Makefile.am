###########################################################################
## Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               ##
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

nobase_include_HEADERS = dimension.h                                           \
                         dimension/array.h                                     \
                         dimension/camera.h                                    \
                         dimension/cameras.h                                   \
                         dimension/canvas.h                                    \
                         dimension/color.h                                     \
                         dimension/error.h                                     \
                         dimension/finishes.h                                  \
                         dimension/geometry.h                                  \
                         dimension/gl.h                                        \
                         dimension/light.h                                     \
                         dimension/lights.h                                    \
                         dimension/object.h                                    \
                         dimension/objects.h                                   \
                         dimension/pigments.h                                  \
                         dimension/png.h                                       \
                         dimension/progress.h                                  \
                         dimension/raytrace.h                                  \
                         dimension/scene.h                                     \
                         dimension/texture.h

lib_LTLIBRARIES = libdimension.la

libdimension_la_SOURCES = $(nobase_include_HEADERS)                            \
                          ambient.c                                            \
                          camera.c                                             \
                          canvas.c                                             \
                          color.c                                              \
                          cube.c                                               \
                          diffuse.c                                            \
                          dimension_impl.h                                     \
                          error.c                                              \
                          finish_combination.c                                 \
                          geometry.c                                           \
                          gl.c                                                 \
                          inlines.c                                            \
                          kD_splay_tree.c                                      \
                          kD_splay_tree.h                                      \
                          light.c                                              \
                          object.c                                             \
                          perspective.c                                        \
                          phong.c                                              \
                          png.c                                                \
                          point_light.c                                        \
                          progress.c                                           \
                          raytrace.c                                           \
                          reflective.c                                         \
                          scene.c                                              \
                          solid_pigment.c                                      \
                          sphere.c                                             \
                          texture.c
libdimension_la_LDFLAGS = -version-info 0:0:0
libdimension_la_LIBADD  = -lm -lpthread -lpng -lGL

if PGO

noinst_LTLIBRARIES = libdimension-pgo.la
libdimension_pgo_la_SOURCES = $(libdimension_la_SOURCES)
libdimension_pgo_la_LIBADD  = $(libdimension_la_LIBADD) -lgcov
libdimension_pgo_la_CFLAGS  = -fprofile-generate=$$(readlink -f $(builddir))/pgo

libdimension_la_CFLAGS = -fprofile-correction -fprofile-use=./pgo

all-local: libdimension.la
	@for Plo in .deps/libdimension_la*.Plo; do \
	  [ -f "pgo/.libs/$$(basename $$Plo .Plo).gcda" ] && sed -ri 's|^(libdimension_la-.*).lo: ([^ ])|\1.lo:  pgo/.libs/\1.gcda \2|g' "$$Plo"; \
	  true; \
	done

clean-local:
	rm -rf pgo
endif
