#!/bin/sh

#########################################################################
# Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          #
#                                                                       #
# This file is part of The Dimension Benchmark Suite.                   #
#                                                                       #
# The Dimension Benchmark Suite is free software; you can redistribute  #
# it and/or modify it under the terms of the GNU General Public License #
# as published by the Free Software Foundation; either version 3 of the #
# License, or (at your option) any later version.                       #
#                                                                       #
# The Dimension Benchmark Suite is distributed in the hope that it will #
# be useful, but WITHOUT ANY WARRANTY; without even the implied         #
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See #
# the GNU General Public License for more details.                      #
#                                                                       #
# You should have received a copy of the GNU General Public License     #
# along with this program.  If not, see <http://www.gnu.org/licenses/>. #
#########################################################################

echo -e "Single-threaded"
time ${top_builddir}/dimension/dimension -w1920 -h1080 --quality=1 --threads=1 bench.pov
echo -e "\nMulti-threaded"
time ${top_builddir}/dimension/dimension -w1920 -h1080 --quality=1 bench.pov
