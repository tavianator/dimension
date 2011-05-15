#!/bin/sh

#########################################################################
# Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     #
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

numeric=$(${top_builddir}/dimension/dimension ${dimension_flags} --tokenize ${srcdir}/numeric.pov)
numeric_exp=$(echo -n \
'((integer "1")
  (integer "123456789")
  (integer "01234567")
  (integer "0x123456789")
  - (integer "0x01")

  (float ".1")
  (float "0.1")
  (float "1.0")
  (float "0.123456789")
  - (float "0.123456789")

  < (integer "1") , (float "2.2") , - (float "3.03") >)' \
| tr '\n' ' ' | sed -r 's/[[:space:]]+/ /g')

if [ "$numeric" != "$numeric_exp" ]; then
  echo "numeric.pov tokenized as \"$numeric\"" >&2
  echo "             -- expected \"$numeric_exp\"" >&2
  exit 1
fi