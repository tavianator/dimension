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

INCLUDES = -I$(top_srcdir)/libdimension

check_LTLIBRARIES = libdimension-tests.la
check_PROGRAMS    = error-test                                                 \
                    warning-test                                               \
                    list-test                                                  \
                    polynomial-test                                            \
                    prtree-test                                                \
                    png-test                                                   \
                    gl-test                                                    \
                    render-test                                                \
                    cxx-test
TESTS             = $(check_PROGRAMS)
XFAIL_TESTS       = error-test

if !PNG
XFAIL_TESTS += png-test
endif

if !GL
XFAIL_TESTS += gl-test
endif

libdimension_tests_la_SOURCES = tests.h
libdimension_tests_la_LIBADD  = $(top_builddir)/libdimension/libdimension.la

if GLX
libdimension_tests_la_SOURCES += display.c
libdimension_tests_la_LIBADD += -lX11 -lGL
else
libdimension_tests_la_SOURCES += display-stubs.c
endif

error_test_SOURCES = error.c
error_test_LDADD   = libdimension-tests.la

warning_test_SOURCES = warning.c
warning_test_LDADD   = libdimension-tests.la

list_test_SOURCES = list.c
list_test_LDADD   = libdimension-tests.la

polynomial_test_SOURCES = polynomial.c
polynomial_test_LDADD   = libdimension-tests.la

prtree_test_SOURCES = prtree.c
prtree_test_LDADD   = libdimension-tests.la

png_test_SOURCES = png.c
png_test_LDADD   = libdimension-tests.la

gl_test_SOURCES = gl.c
gl_test_LDADD   = libdimension-tests.la

render_test_SOURCES = render.c
render_test_LDADD   = libdimension-tests.la

cxx_test_SOURCES = cxx.cpp
cxx_test_LDADD   = libdimension-tests.la

clean-local:
	rm -f *.png
