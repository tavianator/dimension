###########################################################################
## Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     ##
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
SUBDIRS = .                                                                    \
          libdimension                                                         \
          libdimension-python                                                  \
          dimension

EXTRA_DIST = autogen.sh

# Set up ./dimension as a python package for tests
$(abs_builddir)/dimension/__init__.py:
	ln -sf "$(abs_top_srcdir)/dimension/__init__.py" dimension/__init__.py
$(abs_builddir)/dimension/wrapper.so:
	ln -sf ../libdimension-python/.libs/wrapper.so dimension/wrapper.so
all-local: $(abs_builddir)/dimension/__init__.py $(abs_builddir)/dimension/wrapper.so

bench:
	cd libdimension && $(MAKE) $(AM_MAKEFLAGS) bench

doc:
	cd libdimension && $(MAKE) $(AM_MAKEFLAGS) doc
	cd libdimension-python && $(MAKE) $(AM_MAKEFLAGS) doc

.PHONY: bench doc
