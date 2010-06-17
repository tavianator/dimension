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

transformations=$(${top_builddir}/dimension/dimension --parse ${srcdir}/transformations.pov)
transformations_exp="$(echo -n \
'((sphere
    (vector (integer 0) (integer 0) (integer 0) (integer 0) (integer 0))
    (integer 0)
    (object-modifiers
      (transformation
        (translation
          (vector (integer -1) (integer 0) (integer 0)
                  (integer 0) (integer 0))))
      (transformation
        (rotation
          (vector (integer 0) (integer 90) (integer 0)
                  (integer 0) (integer 0))))
      (transformation
        (scale
          (vector (integer 2) (integer 2) (integer 2) (integer 2) (integer 2))))
      (transformation
        (translation
          (vector (integer 0) (integer 0) (integer 1) (integer 0) (integer 0))))
      (transformation
        (matrix
           (integer 1) (integer 1) (integer 0)
           (integer 0) (integer 1) (integer 0)
           (integer 0) (integer 0) (integer 1)
           (integer 0) (integer 0) (integer 0)))
      (transformation
        (translation
          (vector (integer 0) (integer 0) (integer 1) (integer 0) (integer 0)))
        (rotation
          (vector (integer 0) (integer 0) (integer 45)
                  (integer 0) (integer 0)))
        inverse))))' \
| tr '\n' ' ' | sed -r 's/[[:space:]]+/ /g')"

if [ "$transformations" != "$transformations_exp" ]; then
  echo "transformations.pov parsed as \"$transformations\"" >&2
  echo "                  -- expected \"$transformations_exp\"" >&2
  exit 1
fi

${top_builddir}/dimension/dimension -w1 -h1 -o /dev/null ${srcdir}/transformations.pov
