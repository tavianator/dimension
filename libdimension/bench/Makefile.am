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

EXTRA_PROGRAMS = array.bench                                                   \
                 geometry.bench                                                \
                 polynomial.bench                                              \
                 future.bench                                                  \
                 prtree.bench                                                  \
                 triangle.bench

AM_CFLAGS  = $(libsandglass_CFLAGS) -fno-inline -I$(top_srcdir)/libdimension
AM_LDFLAGS = $(libsandglass_LIBS) $(top_builddir)/libdimension/libdimension.la

array_bench_SOURCES = array.c
geometry_bench_SOURCES = geometry.c
polynomial_bench_SOURCES = polynomial.c
future_bench_SOURCES = future.c
prtree_bench_SOURCES = prtree.c
prtree_bench_CFLAGS = $(AM_CFLAGS) -finline
triangle_bench_SOURCES = triangle.c

bench: $(EXTRA_PROGRAMS)
	./array.bench
	./geometry.bench
	./polynomial.bench
	./future.bench
	./prtree.bench
	./triangle.bench

clean-local:
	rm -f *.bench

.PHONY: bench
