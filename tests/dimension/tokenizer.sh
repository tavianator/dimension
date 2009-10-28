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

exitstatus=0

punctuation=$(${top_builddir}/dimension/dimension --tokenize ${srcdir}/punctuation.pov)
punctuation_exp='({ \( [ < + - * / , > ] \) })'

if [ "$punctuation" != "$punctuation_exp" ]; then
  echo "punctuation.pov tokenized as \"$punctuation\"" >&2
  echo "                 -- expected \"$punctuation_exp\"" >&2
  exitstatus=1
fi

numeric=$(${top_builddir}/dimension/dimension --tokenize ${srcdir}/numeric.pov)
numeric_exp='((int "1") (int "123456789") (int "01234567") (int "0x123456789") - (int "0x01") (float ".1") (float "0.1") (float "1.0") (float "0.123456789") - (float "0.123456789"))'

if [ "$numeric" != "$numeric_exp" ]; then
  echo "numeric.pov tokenized as \"$numeric\"" >&2
  echo "             -- expected \"$numeric_exp\"" >&2
  exitstatus=1
fi

exit $exitstatus