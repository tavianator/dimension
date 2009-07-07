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

check_LTLIBRARIES = libdimension-tests.la                                      \
                    libdimensionxx-tests.la
check_PROGRAMS    = warning-test                                               \
                    error-test                                                 \
                    png-test                                                   \
                    pngxx-test                                                 \
                    raytrace-test                                              \
                    raytracexx-test                                            \
                    gl-test                                                    \
                    glxx-test
TESTS             = $(check_PROGRAMS)
XFAIL_TESTS       = error-test

INCLUDES = -I../libdimension -I../libdimensionxx

libdimension_tests_la_SOURCES = tests.h                                        \
                                tests.c
libdimension_tests_la_LIBADD  = ../libdimension/libdimension.la

libdimensionxx_tests_la_SOURCES = testsxx.hpp                                  \
                                  testsxx.cpp
libdimensionxx_tests_la_LIBADD  = ./libdimension-tests.la                      \
                                  ../libdimensionxx/libdimensionxx.la

warning_test_SOURCES = warning.c
warning_test_LDADD   = ./libdimension-tests.la

error_test_SOURCES = error.c
error_test_LDADD   = ./libdimension-tests.la

png_test_SOURCES = png.c
png_test_LDADD   = ./libdimension-tests.la

pngxx_test_SOURCES = pngxx.cpp
pngxx_test_LDADD   = ./libdimensionxx-tests.la

raytrace_test_SOURCES = raytrace.c
raytrace_test_LDADD   = ./libdimension-tests.la

raytracexx_test_SOURCES = raytracexx.cpp
raytracexx_test_LDADD   = ./libdimensionxx-tests.la

gl_test_SOURCES = gl.c
gl_test_LDADD   = ./libdimension-tests.la

glxx_test_SOURCES = glxx.cpp
glxx_test_LDADD   = ./libdimensionxx-tests.la
