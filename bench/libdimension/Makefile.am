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

EXTRA_PROGRAMS = bench-array bench-geometry bench-bvst

AM_CFLAGS = $(libsandglass_CFLAGS) -fno-inline
AM_LDFLAGS = $(libsandglass_LIBS) $(top_builddir)/libdimension/libdimension.la

bench_array_SOURCES    = array.c
bench_geometry_SOURCES = geometry.c
bench_bvst_SOURCES     = bvst.c

bench: bench-array$(EXEEXT) bench-geometry$(EXEEXT) bench-bvst$(EXEEXT)
	./bench-array$(EXEEXT)
	./bench-geometry$(EXEEXT)
	./bench-bvst$(EXEEXT)

.PHONY: bench
