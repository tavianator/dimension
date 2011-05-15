#!/bin/sh

#########################################################################
# Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     #
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

csg=$(${top_builddir}/dimension/dimension ${dimension_flags} -w768 -h480 --parse ${srcdir}/csg.pov)
csg_exp="$(echo -n \
'((camera
    perspective
    (location (vector (integer 0) (integer 0) (integer -4)
                      (integer 0) (integer 0)))
    (right (vector (float 1.6) (integer 0) (integer 0) (integer 0) (integer 0)))
    (look_at (vector (integer 0) (integer 0) (integer 0)
                     (integer 0) (integer 0))))
  (background
    (vector (integer 0) (float 0.1) (float 0.2) (float 0.1) (integer 0)))
  (object
    (union
      (object
        (sphere
          (vector (float -1.5) (float 0) (float 0) (float 0) (float 0))
          (integer 1))
        (object-modifiers
          (pigment
            (vector (integer 1) (integer 0) (integer 0)
                    (integer 0) (integer 0))
            pigment-modifiers))))
    object-modifiers)
  (object
    (union
      (light_source
        (vector (integer 0) (integer 20) (integer 0) (integer 0) (integer 0))
        (vector (float 0.5) (float 0.5) (float 0.5) (integer 0) (integer 0))
        object-modifiers))
    object-modifiers)
  (object
    (difference
      (light_source
        (vector (integer -15) (integer 0) (integer 0) (integer 0) (integer 0))
        (vector (float 0.5) (float 0.5) (float 0.5) (integer 0) (integer 0))
        object-modifiers)
      (object
        (sphere
          (vector (float 1.5) (float -20) (float 0) (float 0) (float 0))
          (integer 1))
        (object-modifiers
          (pigment
            (vector (integer 0) (integer 1) (integer 0)
                    (integer 0) (integer 0))
            pigment-modifiers)))
      (light_source
        (vector (integer 15) (integer 0) (integer 0) (integer 0) (integer 0))
        (vector (float 0.5) (float 0.5) (float 0.5) (integer 0) (integer 0))
        object-modifiers)
      (object
        (box
          (vector (float 0.7) (float -20.8) (float -0.8)
                  (integer 0) (integer 0))
          (vector (float 2.3) (float -19.2) (float 0.8)
                  (integer 0) (integer 0)))
        (object-modifiers
          (pigment
            (vector (integer 0) (integer 0) (integer 1)
                    (integer 0) (integer 0))
            pigment-modifiers))))
    (object-modifiers
      (transformation
        (translation (vector (integer 0) (integer 20) (integer 0)
                             (integer 0) (integer 0)))))))' \
| tr '\n' ' ' | sed -r 's/[[:space:]]+/ /g')"

if [ "$csg" != "$csg_exp" ]; then
  echo "csg.pov parsed as \"$csg\"" >&2
  echo "      -- expected \"$csg_exp\"" >&2
  exit 1
fi

${top_builddir}/dimension/dimension ${dimension_flags} -w768 -h480 -o csg.png ${srcdir}/csg.pov
