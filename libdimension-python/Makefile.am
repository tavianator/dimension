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

SUBDIRS = .                                                                    \
          tests

INCLUDES = -I$(top_srcdir)/libdimension

AM_CFLAGS = $(Python_CFLAGS)
AM_LDFLAGS = $(Python_LDFLAGS)

pyexec_LTLIBRARIES = dimension.la
dimension_la_SOURCES = Matrix.c                                                \
                       Scene.c                                                 \
                       Vector.c                                                \
                       dimension.c
dimension_la_LDFLAGS = -avoid-version -module
dimension_la_LIBADD  = $(top_builddir)/libdimension/libdimension.la
