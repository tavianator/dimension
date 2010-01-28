###########################################################################
## Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               ##
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

INCLUDES = -I$(top_srcdir)/libdimension

check_LTLIBRARIES = libdimension-tests.la
check_PROGRAMS    = error-test                                                 \
                    warning-test                                               \
                    bvst-test                                                  \
                    png-test                                                   \
                    gl-test                                                    \
                    cxx-test
TESTS             = $(check_PROGRAMS)
XFAIL_TESTS       = error-test

.PHONY: clean-local-png-test
clean-local: clean-local-png-test

libdimension_tests_la_SOURCES = tests.c tests.h

if PGO
libdimension_tests_la_LIBADD = $(top_builddir)/libdimension/libdimension-pgo.la
else
libdimension_tests_la_LIBADD = $(top_builddir)/libdimension/libdimension.la
endif

error_test_SOURCES = error.c
error_test_LDADD   = libdimension-tests.la

warning_test_SOURCES = warning.c
warning_test_LDADD   = libdimension-tests.la

bvst_test_SOURCES = bvst.c
bvst_test_LDADD   = libdimension-tests.la

png_test_SOURCES = png.c
png_test_LDADD   = libdimension-tests.la
clean-local-png-test:
	rm -f *.png

gl_test_SOURCES = gl.c
gl_test_LDADD   = libdimension-tests.la

cxx_test_SOURCES = cxx.cpp
cxx_test_LDADD   = libdimension-tests.la
