###########################################################################
## Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               ##
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
                         dimension/geometry.h                                  \
                         dimension/gl.h                                        \
                         dimension/png.h                                       \
                         dimension/progress.h                                  \
                         dimension/object.h                                    \
                         dimension/objects.h                                   \
                         dimension/raytrace.h                                  \
                         dimension/scene.h

lib_LTLIBRARIES = libdimension.la

libdimension_la_SOURCES = $(nobase_include_HEADERS)                            \
                          camera.c                                             \
                          cameras.c                                            \
                          canvas.c                                             \
                          color.c                                              \
                          error.c                                              \
                          geometry.c                                           \
                          gl.c                                                 \
                          inlines.c                                            \
                          png.c                                                \
                          progress.c                                           \
                          object.c                                             \
                          objects.c                                            \
                          raytrace.c                                           \
                          scene.c
libdimension_la_LDFLAGS = -version-info 0:0:0
libdimension_la_LIBADD  = -lm -lpthread -lpng -lGL
