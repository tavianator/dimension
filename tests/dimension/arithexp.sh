#!/bin/sh

#########################################################################
# Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               #
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

arithexp=$(${top_builddir}/dimension/dimension --parse ${srcdir}/arithexp.pov)
arithexp_exp="$(echo -n \
'((sphere
    (vector (float 2) (float 2) (float 3) (integer 0) (integer 0))
    (float 0.718282)
    object-modifiers))' \
| tr '\n' ' ' | sed -r 's/[[:space:]]+/ /g')"

if [ "$arithexp" != "$arithexp_exp" ]; then
  echo "arithexp.pov parsed as \"$arithexp\"" >&2
  echo "           -- expected \"$arithexp_exp\"" >&2
  exit 1
fi
