###########################################################################
## Copyright (C) 2011-2014 Tavian Barnes <tavianator@tavianator.com>     ##
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

SUBDIRS = .                                                                    \
          tests

AM_CFLAGS = $(Python_CFLAGS) -I$(top_srcdir)/libdimension
AM_LDFLAGS = $(Python_LDFLAGS)

dimension.c: dimension.pyx dimension.pxd
if DEBUG
	cython --line-directives --cleanup 2 dimension.pyx
else
	cython --cleanup 2 dimension.pyx
endif
	sed -i 's/initdimension/initwrapper/g' dimension.c
	sed -i 's/PyInit_dimension/PyInit_wrapper/g' dimension.c

pkgpyexec_LTLIBRARIES = wrapper.la
wrapper_la_SOURCES = dimension.c                                               \
                     platform.c                                                \
                     platform.h
wrapper_la_LDFLAGS = -avoid-version -module
wrapper_la_LIBADD  = $(top_builddir)/libdimension/libdimension.la

dimension.$(OBJEXT): CFLAGS += -Wno-shadow -Wno-sign-compare
dimension.lo: CFLAGS += -Wno-shadow -Wno-sign-compare

EXTRA_DIST = dimension.pyx                                                     \
             dimension.pxd

doc:
	mkdir -p doc
	PYTHONPATH=$(top_builddir) pydoc -w dimension
	mv dimension.html doc/
clean-doc:
	rm -rf doc

.PHONY: doc
