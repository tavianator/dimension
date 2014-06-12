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

AM_CFLAGS = -I$(top_srcdir)/libdimension
AM_CXXFLAGS = $(AM_CFLAGS)

check_LTLIBRARIES = libdimension-tests.la                                      \
                    libdimension-unit-test.la
check_PROGRAMS    = warning.test                                               \
                    warning-as-error.test                                      \
                    error.test                                                 \
                    custom-error-fn.test                                       \
                    pool.test                                                  \
                    dictionary.test                                            \
                    polynomial.test                                            \
                    prtree.test                                                \
                    png.test                                                   \
                    gl.test                                                    \
                    render.test                                                \
                    c89.test                                                   \
                    c99.test                                                   \
                    cxx.test
TESTS             = $(check_PROGRAMS)
XFAIL_TESTS       = warning-as-error.test error.test

render.log: png.log

if !PNG
XFAIL_TESTS += png.test
endif

if !GL
XFAIL_TESTS += gl.test
endif

libdimension_tests_la_SOURCES = test_canvas.c                                  \
                                tests.h
libdimension_tests_la_LIBADD  = $(top_builddir)/libdimension/libdimension.la

if GLX
libdimension_tests_la_SOURCES += display.c
libdimension_tests_la_LIBADD += -lX11 -lGL
else
libdimension_tests_la_SOURCES += display-stubs.c
endif

libdimension_unit_test_la_SOURCES = unit-test.c
libdimension_unit_test_la_CFLAGS  = $(CHECK_CFLAGS) $(AM_CFLAGS)
libdimension_unit_test_la_LIBADD  = $(CHECK_LIBS) $(top_builddir)/libdimension/libdimension.la

warning_test_SOURCES = warning.c
warning_test_LDADD   = libdimension-tests.la

warning_as_error_test_SOURCES = warning-as-error.c
warning_as_error_test_LDADD   = libdimension-tests.la

error_test_SOURCES = error.c
error_test_LDADD   = libdimension-tests.la

custom_error_fn_test_SOURCES = custom-error-fn.c
custom_error_fn_test_LDADD   = libdimension-tests.la

pool_test_SOURCES = pool.c
pool_test_LDADD   = libdimension-unit-test.la

dictionary_test_SOURCES = dictionary.c
dictionary_test_LDADD   = libdimension-unit-test.la

polynomial_test_SOURCES = polynomial.c
polynomial_test_LDADD   = libdimension-unit-test.la

prtree_test_SOURCES = prtree.c
prtree_test_LDADD   = libdimension-tests.la

png_test_SOURCES = png.c
png_test_LDADD   = libdimension-tests.la

gl_test_SOURCES = gl.c
gl_test_LDADD   = libdimension-tests.la

render_test_SOURCES = render.c
render_test_LDADD   = libdimension-tests.la

c89_test_SOURCES = c89.c
c89_test_LDADD   = libdimension-tests.la

c89.o: CFLAGS += -std=c89 -Wpedantic

c99_test_SOURCES = c99.c
c99_test_LDADD   = libdimension-tests.la

c99.o: CFLAGS += -std=c99 -Wpedantic

cxx_test_SOURCES = cxx.cpp
cxx_test_LDADD   = libdimension-tests.la

clean-local:
	rm -f *.png
