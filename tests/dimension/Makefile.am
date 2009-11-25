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

TESTS = punctuation.sh numeric.sh strings.sh labels.sh directives.sh arithexp.sh demo.sh
TESTS_ENVIRONMENT = top_builddir=$(top_builddir)

.sh:
	cp $(srcdir)/$@ .

EXTRA_DIST = $(TESTS)                                                          \
             punctuation.pov                                                   \
             numeric.pov                                                       \
             strings.pov                                                       \
             labels.pov                                                        \
             directives.pov                                                    \
             arithexp.pov                                                      \
             demo.pov

clean-local:
	rm -f *.png
