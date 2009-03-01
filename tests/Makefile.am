###########################################################################
## Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               ##
##                                                                       ##
## This file is part of The FPFD Library Build Suite.                    ##
##                                                                       ##
## The FPFD Library Build Suite is free software; you can redistribute   ##
## it and/or modify it under the terms of the GNU General Public License ##
## as published by the Free Software Foundation; either version 3 of the ##
## License, or (at your option) any later version.                       ##
##                                                                       ##
## The FPFD Library Build Suite is distributed in the hope that it will  ##
## be useful, but WITHOUT ANY WARRANTY; without even the implied         ##
## warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See ##
## the GNU General Public License for more details.                      ##
##                                                                       ##
## You should have received a copy of the GNU General Public License     ##
## along with this program.  If not, see <http://www.gnu.org/licenses/>. ##
###########################################################################

check_PROGRAMS = png
TESTS          = $(check_PROGRAMS)

INCLUDES = -I../libdimension -I../libdimension-png

png_SOURCES = png.c
png_LDADD   = ../libdimension/libdimension.la ../libdimension-png/libdimension-png.la
