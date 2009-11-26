#!/bin/sh

#########################################################################
# Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               #
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

arithexp=$(${top_builddir}/dimension/dimension --tokenize --parse ${srcdir}/arithexp.pov)
arithexp_exp="$(echo -n \
'(sphere {
    (integer "2") *
    < < (float "2.0") - (float "1.0") , (float "3.0") , (float "4.0") > . x ,
      \( (float "1.0") + (integer "2") \) * (integer "2") - (integer "5") ,
      (float "1.0") + (integer "2") * (integer "2") - (integer "4") >
    -
    - < (integer "0") , (integer "0") , (integer "1") > ,
    (float "2.25") - (integer "1") * (integer "2")
  })' \
| tr '\n' ' ' | sed -r 's/[[:space:]]+/ /g')
$(echo -n \
'((sphere
    (vector (float 2) (float 2) (float 3) (integer 0) (integer 0))
    (float 0.25)
    object-modifiers))' \
| tr '\n' ' ' | sed -r 's/[[:space:]]+/ /g')"

if [ "$arithexp" != "$arithexp_exp" ]; then
  echo "arithexp.pov parsed as \"$arithexp\"" >&2
  echo "           -- expected \"$arithexp_exp\"" >&2
  exit 1
fi
