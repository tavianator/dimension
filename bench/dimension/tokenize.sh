#!/bin/bash

#########################################################################
# Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     #
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

for i in {1..10000}; do
  echo '{}()[]+-*/,;?:&.|=<>!<= >= != "This is a string with escape sequences: \a\b\f\n\r\t\u2123\v\\\"" 1 123456789 01234567 0x123456789 -0x01 .1 0.1 1.0 0.123456789 -0.123456789 <1, 2.2, -3.03> Undefined'
done | (time ${top_builddir}/dimension/dimension --tokenize /dev/stdin >/dev/null)
