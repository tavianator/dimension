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

nobase_include_HEADERS = dimensionxx.hpp                                       \
                         dimensionxx/array.hpp                                 \
                         dimensionxx/canvas.hpp                                \
                         dimensionxx/camera.hpp                                \
                         dimensionxx/cameras.hpp                               \
                         dimensionxx/color.hpp                                 \
                         dimensionxx/cookie.hpp                                \
                         dimensionxx/geometry.hpp                              \
                         dimensionxx/gl.hpp                                    \
                         dimensionxx/object.hpp                                \
                         dimensionxx/objects.hpp                               \
                         dimensionxx/png.hpp                                   \
                         dimensionxx/progress.hpp                              \
                         dimensionxx/raytrace.hpp                              \
                         dimensionxx/scene.hpp

INCLUDES = -I../libdimension

lib_LTLIBRARIES = libdimensionxx.la

libdimensionxx_la_SOURCES = $(nobase_include_HEADERS)                          \
                            canvas.cpp                                         \
                            camera.cpp                                         \
                            cameras.cpp                                        \
                            color.cpp                                          \
                            gl.cpp                                             \
                            error.cpp                                          \
                            object.cpp                                         \
                            objects.cpp                                        \
                            png.cpp                                            \
                            progress.cpp                                       \
                            raytrace.cpp                                       \
                            scene.cpp

if FOPENCOOKIE
  libdimensionxx_la_SOURCES += cookie-fopencookie.cpp
else
  libdimensionxx_la_SOURCES += cookie-tmpfile.cpp
endif

libdimensionxx_la_LDFLAGS = -version-info 0:0:0
libdimensionxx_la_LIBADD  = ../libdimension/libdimension.la
