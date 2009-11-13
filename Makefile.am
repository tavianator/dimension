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

.PHONY: pgo bench
pgo:
	@if [ ! -d $(top_builddir)/libdimension/pgo/.libs ] || [ -z "$$(ls $(top_builddir)/libdimension/pgo/.libs/*.gcda 2>/dev/null)" ]; then \
	  $(MAKE) $(AM_MAKEFLAGS) -C libdimension libdimension-pgo.la && \
	  $(MAKE) -j1 -C tests/libdimension check; \
	fi
	@rename _pgo_la _la $$(find $(top_builddir)/libdimension/pgo/.libs/)

all-recursive check-recursive install-recursive installcheck-recursive distdir: pgo

else  ## PGO
.PHONY: bench
endif ## PGO

bench: all-recursive
	cd bench && $(MAKE) $(AM_MAKEFLAGS) bench
