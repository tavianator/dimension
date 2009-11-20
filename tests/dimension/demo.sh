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

demo=$(${top_builddir}/dimension/dimension --tokenize --parse ${srcdir}/demo.pov)
demo_exp='(box { < - (float "2.0") + (integer "1") , - (integer "2") / (float "2.0") , - (float "1.0") > , < (float "1.0") , \( (float "1.0") + (integer "2") \) * (integer "2") - (integer "5") , (float "1.0") + (integer "2") * (integer "2") - (integer "4") > } sphere { < (integer "1") / (integer "5") + - (integer "1") / (integer "5") , (integer "1") / (float "5.0") - (float "1.0") / (integer "5") , - (integer "1") - - (integer "1") > , (float "1.25") })
((box (vector (float -1) (float -1) (float -1)) (vector (float 1) (float 1) (float 1))) (sphere (vector (float 0) (float 0) (integer 0)) (float 1.25)))'

if [ "$demo" != "$demo_exp" ]; then
  echo "demo.pov parsed as \"$demo\"" >&2
  echo "       -- expected \"$demo_exp\"" >&2
  exit 1
fi

${top_builddir}/dimension/dimension -o demo.png ${srcdir}/demo.pov
