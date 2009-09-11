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

check_LTLIBRARIES = libdimension-tests.la
check_PROGRAMS    = warning-test                                               \
                    error-test                                                 \
                    png-test                                                   \
                    gl-test
TESTS             = $(check_PROGRAMS)
XFAIL_TESTS       = error-test

INCLUDES = -I../libdimension -I../libdimensionxx

libdimension_tests_la_SOURCES = tests.hpp                                      \
                                tests.cpp
libdimension_tests_la_LIBADD  = ../libdimensionxx/libdimensionxx.la

warning_test_SOURCES = warning.cpp
warning_test_LDADD   = ./libdimension-tests.la

error_test_SOURCES = error.cpp
error_test_LDADD   = ./libdimension-tests.la

png_test_SOURCES = png.cpp
png_test_LDADD   = ./libdimension-tests.la

gl_test_SOURCES = gl.cpp
gl_test_LDADD   = ./libdimension-tests.la
