###########################################################################
## Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               ##
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

check_PROGRAMS = warning error png pngxx
TESTS          = $(check_PROGRAMS)
XFAIL_TESTS    = error

INCLUDES = -I../libdimension -I../libdimensionxx

warning_SOURCES = warning.c
warning_LDADD   = ../libdimension/libdimension.la

error_SOURCES = error.c
error_LDADD   = ../libdimension/libdimension.la

png_SOURCES = png.c
png_LDADD   = ../libdimension/libdimension.la

pngxx_SOURCES = pngxx.cpp
pngxx_LDADD   = ../libdimensionxx/libdimensionxx.la
