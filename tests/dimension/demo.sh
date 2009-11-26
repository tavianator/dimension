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

demo=$(${top_builddir}/dimension/dimension --parse ${srcdir}/demo.pov)
demo_exp=$(echo -n \
'((background
    (vector (integer 0) (integer 0) (float 0.1) (float 0.1) (integer 0)))
  (light_source
    (vector (integer -15) (integer 20) (integer 10) (integer 0) (integer 0))
    (vector (integer 1) (integer 1) (integer 1) (integer 0) (integer 0)))
  (box
    (vector (integer -1) (integer -1) (integer -1) (integer 0) (integer 0))
    (vector (integer 1) (integer 1) (integer 1) (integer 0) (integer 0))
    (object-modifiers
      (rotate (vector (integer 45) (integer 0) (integer 0)
                      (integer 0) (integer 0)))
      (texture
        (pigment (vector (integer 0) (integer 0) (integer 1)
                         (float 0.25) (float 0.25))))))
  (sphere
    (vector (integer 0) (integer 0) (integer 0) (integer 0) (integer 0))
    (float 1.25)
    (object-modifiers
      (texture
        (pigment (vector (integer 0) (integer 1) (integer 0)
                         (integer 0) (integer 0)))))))' \
| tr '\n' ' ' | sed -r 's/[[:space:]]+/ /g')

if [ "$demo" != "$demo_exp" ]; then
  echo "demo.pov parsed as \"$demo\"" >&2
  echo "       -- expected \"$demo_exp\"" >&2
  exit 1
fi

${top_builddir}/dimension/dimension -o demo.png ${srcdir}/demo.pov
