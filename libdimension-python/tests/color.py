#!/usr/bin/python3

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

from dimension import *

# Treat warnings as errors for tests
die_on_warnings(True)

c = Color(0, 0.5, 1, trans = 0.5, filter = 0.35)
assert repr(c) == 'dimension.Color(0.0, 0.5, 1.0, 0.5, 0.35)', repr(c)
assert str(c) == '<red = 0.0, green = 0.5, blue = 1.0, \
trans = 0.5, filter = 0.35>', str(c)
assert c.red    == 0,    c.red
assert c.green  == 0.5,  c.green
assert c.blue   == 1,    c.blue
assert c.trans  == 0.5,  c.filter
assert c.filter == 0.35, c.trans

c = Color(1, 0.5, 0)
assert str(c) == '<red = 1.0, green = 0.5, blue = 0.0>', str(c)

assert Black   == Color(0, 0, 0), Black
assert White   == Color(1, 1, 1), White
assert Clear   == Color(0, 0, 0, trans = 1), Clear
assert Red     == Color(1, 0, 0), Red
assert Green   == Color(0, 1, 0), Green
assert Blue    == Color(0, 0, 1), Blue
assert Magenta == Color(1, 0, 1), Magenta
assert Orange  == Color(1, 0.5, 0), Orange
assert Yellow  == Color(1, 1, 0), Yellow
assert Cyan    == Color(0, 1, 1), Cyan

assert White, bool(White)
assert not Black, not Black

assert Red + Blue == Magenta, Red + Blue
assert 0.5*White == Color(0.5, 0.5, 0.5), 0.5*White
