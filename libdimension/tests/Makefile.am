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

check_LTLIBRARIES = libdimension-tests.la \
  libdimension-unit-test.la
check_PROGRAMS    = modules.test \
  c89.test \
  c99.test \
  c++03.test \
  c++11.test \
  warning.test \
  warning-as-error.test \
  error.test \
  custom-error-fn.test \
  pool.test \
  dictionary.test \
  polynomial.test \
  prtree.test \
  future.test \
  png.test \
  gl.test \
  render.test
TESTS             = $(check_PROGRAMS)
XFAIL_TESTS       = warning-as-error.test error.test

render.log: png.log

if !PNG
XFAIL_TESTS += png.test
endif

if !GL
XFAIL_TESTS += gl.test
endif

libdimension_tests_la_SOURCES = test_canvas.c \
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

modules_test_SOURCES = build/modules/base.c \
  build/modules/canvas.c \
  build/modules/color.c \
  build/modules/concurrency.c \
  build/modules/math.c \
  build/modules/model.c \
  build/modules/pattern.c \
  build/modules/platform.c \
  build/modules/render.c
modules_test_LDADD   = libdimension-unit-test.la

c89_test_SOURCES = build/c89.c
c89_test_LDADD   = libdimension-tests.la

c89.o: CFLAGS += -std=c89 -Wpedantic

c99_test_SOURCES = build/c99.c
c99_test_LDADD   = libdimension-tests.la

c99.o: CFLAGS += -std=c99 -Wpedantic

c__03_test_SOURCES = build/c++03.cpp
c__03_test_LDADD   = libdimension-tests.la

c++03.o: CFLAGS += -std=c++03 -Wpedantic

c__11_test_SOURCES = build/c++11.cpp
c__11_test_LDADD   = libdimension-tests.la

c++11.o: CFLAGS += -std=c++11 -Wpedantic

warning_test_SOURCES = base/error/warning.c
warning_test_LDADD   = libdimension-tests.la

warning_as_error_test_SOURCES = base/error/warning-as-error.c
warning_as_error_test_LDADD   = libdimension-tests.la

error_test_SOURCES = base/error/error.c
error_test_LDADD   = libdimension-tests.la

custom_error_fn_test_SOURCES = base/error/custom-error-fn.c
custom_error_fn_test_LDADD   = libdimension-tests.la

pool_test_SOURCES = base/pool.c
pool_test_LDADD   = libdimension-unit-test.la

dictionary_test_SOURCES = base/dictionary.c
dictionary_test_LDADD   = libdimension-unit-test.la

polynomial_test_SOURCES = math/polynomial.c
polynomial_test_LDADD   = libdimension-unit-test.la

prtree_test_SOURCES = bvh/prtree.c
prtree_test_LDADD   = libdimension-tests.la

future_test_SOURCES = concurrency/future.c
future_test_LDADD   = libdimension-unit-test.la

png_test_SOURCES = canvas/png.c
png_test_LDADD   = libdimension-tests.la

gl_test_SOURCES = canvas/gl.c
gl_test_LDADD   = libdimension-tests.la

render_test_SOURCES = render/render.c
render_test_LDADD   = libdimension-tests.la

clean-local:
	rm -f *.png
