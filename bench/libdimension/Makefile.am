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

INCLUDES = -I$(top_srcdir)/libdimension

EXTRA_PROGRAMS = bench-array bench-geometry bench-kD_splay_tree

bench_array_SOURCES = array.c
bench_array_LDADD   = -lsandglass $(top_builddir)/libdimension/libdimension.la

bench_geometry_SOURCES = geometry.c
bench_geometry_LDADD   = -lsandglass $(top_builddir)/libdimension/libdimension.la

bench_kD_splay_tree_SOURCES = kD_splay_tree.c
bench_kD_splay_tree_LDADD   = -lsandglass $(top_builddir)/libdimension/libdimension.la

bench: bench-array$(EXEEXT) bench-geometry$(EXEEXT) bench-kD_splay_tree$(EXEEXT)
	./bench-array$(EXEEXT)
	./bench-geometry$(EXEEXT)
	./bench-kD_splay_tree$(EXEEXT)

.PHONY: bench
