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

directives=$(${top_builddir}/dimension/dimension --tokenize --parse ${srcdir}/directives.pov)
directives_exp="$(echo -n \
'(#declare (identifier "Center") = (integer "0") ;
  #declare (identifier "R") = (integer "1") ;
  #local (identifier "Color") = rgb < (integer "1") , (integer "0") , (integer "1") > ;
  #declare (identifier "Unused") = - (integer "1") ;
  #undef (identifier "Unused")
  #if \( #if \( (integer "1") = (integer "1") \) (integer "0") #end = (integer "0") & (integer "0") \)
    error (identifier "Illegal")
  #else
    sphere {
      (identifier "Center") , (identifier "R")
      pigment {
        color (identifier "Color") green (integer "1")
      }
    }
  #end)' \
| tr '\n' ' ' | sed -r 's/[[:space:]]+/ /g')
$(echo -n \
'((sphere
    (vector (integer 0) (integer 0) (integer 0) (integer 0) (integer 0))
    (integer 1)
    (object-modifiers
      (texture
        (pigment (color (integer 1) (integer 1) (integer 1)
                        (integer 0) (integer 0)))))))' \
| tr '\n' ' ' | sed -r 's/[[:space:]]+/ /g')"

if [ "$directives" != "$directives_exp" ]; then
  echo "directives.pov parsed as \"$directives\"" >&2
  echo "             -- expected \"$directives_exp\"" >&2
  exit 1
fi
