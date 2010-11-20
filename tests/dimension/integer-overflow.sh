#!/bin/sh

#########################################################################
# Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          #
#                                                                       #
# This file is part of The Dimension Test Suite.                        #
#                                                                       #
# The Dimension Test Suite is free software; you can redistribute it    #
# and/or modify it under the terms of the GNU General Public License as #
# published by the Free Software Foundation; either version 3 of the    #
# License, or (at your option) any later version.                       #
#                                                                       #
# The Dimension Test Suite is distributed in the hope that it will be   #
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty   #
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  #
# General Public License for more details.                              #
#                                                                       #
# You should have received a copy of the GNU General Public License     #
# along with this program.  If not, see <http://www.gnu.org/licenses/>. #
#########################################################################

integer_overflow=$(${top_builddir}/dimension/dimension --parse ${srcdir}/integer-overflow.pov)
integer_overflow_exp="$(echo -n \
'((object
    (torus
      (float 1e+19)
      (float 6.15656e+113))
    object-modifiers))' \
| tr '\n' ' ' | sed -r 's/[[:space:]]+/ /g')"

if [ "$integer_overflow" != "$integer_overflow_exp" ]; then
  echo "integer-overflow.pov parsed as \"$integer_overflow\"" >&2
  echo "                   -- expected \"$integer_overflow_exp\"" >&2
  exit 1
fi
