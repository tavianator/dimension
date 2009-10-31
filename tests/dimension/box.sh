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

box=$(${top_builddir}/dimension/dimension --tokenize --parse ${srcdir}/box.pov)
box_exp='(box { < - (float "0.125") , - (integer "1") , - (integer "1") > , < (float "0.125") , (integer "1") , (integer "1") > } box { < - (integer "1") , - (integer "1") , - (float "0.125") > , < (integer "1") , (integer "1") , (float "0.125") > })
((box (vector (float -0.125) (float -1) (float -1)) (vector (float 0.125) (float 1) (float 1))) (box (vector (float -1) (float -1) (float -0.125)) (vector (float 1) (float 1) (float 0.125))))'

if [ "$box" != "$box_exp" ]; then
  echo "box.pov parsed as \"$box\"" >&2
  echo "      -- expected \"$box_exp\"" >&2
  exit 1
fi

${top_builddir}/dimension/dimension -o box.png ${srcdir}/box.pov
