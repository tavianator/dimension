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

ACLOCAL_AMFLAGS = -I m4
SUBDIRS = libdimension                                                         \
          dimension                                                            \
          tests                                                                \
          bench                                                                \
          doc

EXTRA_DIST = autogen.sh

if PGO
all-recursive:
	cd $(top_builddir)/libdimension && $(MAKE) $(AM_MAKEFLAGS) libdimension-pgo.la
	cd $(top_builddir)/tests/libdimension && $(MAKE) check
	@for gcda in $(top_builddir)/tests/libdimension/pgo/.libs/*_pgo*.gcda; do \
	  mv $$gcda $${gcda/_pgo/}; \
	done
	cd $(top_builddir)/libdimension && $(MAKE) $(AM_MAKEFLAGS) libdimension.la
	cd $(top_builddir)/dimension && $(MAKE) $(AM_MAKEFLAGS) all
	cd $(top_builddir)/doc && $(MAKE) $(AM_MAKEFLAGS) all
endif

bench: all-recursive
	cd bench && $(MAKE) $(AM_MAKEFLAGS) bench

.PHONY: bench
